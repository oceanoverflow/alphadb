#include "raft_server.h"
#include "log.h"

#include <string>
#include <random>
#include <fstream>
#include <utility>
#include <algorithm>

// TODO 
// 1. If election timeout elapses without receiving append_entries
// RPC from current leader or granting vote to candidate: convert_to candidate

raft_server::raft_server(dispatcher* d, short port, int id)
: rpc_server(d, port), id_{id}, apply_func_(0), current_term_(0), voted_for_(-1), commit_index_(0), last_applied_(0)
{
    Register("raft::request_vote", this, &raft_server::request_vote);
    Register("raft::append_entries", this, &raft_server::append_entries);
    rpc_server::start();
}

raft_server::~raft_server()
{
    for(auto& peer : peers_)
        delete peer;
}

void raft_server::start()
{
    read_state_from_disk();
    reschedule_election();
}

void raft_server::shutdown()
{
    persist_state_to_disk();
    
    if (role_ == role::leader) 
        dispatcher_->cancel(heartbeat_timer_id_);
    else if(role_ == role::candidate)
        dispatcher_->cancel(election_timer_id_);
    else
        dispatcher_->cancel(timer_id_);

    rpc_server::shutdown();
}

bool raft_server::replicate(struct log entry)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // must be leader to call this function
        if (role_ != role::leader)
            return false;
        // If command received from client
        // append entry to local log
        entry.index = log_entries_.size();
        entry.term = current_term_;
        log_entries_.push_back(entry);
    }
    // respond after entry applied to state machine
    apply_to_statemachine();
    return true;
}

int raft_server::majority() const
{
    // for example, a majority of 7 peers is (7+1)/2 = 4 peers
    std::lock_guard<std::mutex> lock(mutex_);
    int s = peers_.size();
    return (s + 1) / 2;
}

bool raft_server::is_leader(int term) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    role r = role_;
    return r == role::leader;
}

void raft_server::convert_to(role r, int term)
{
    if (r == role::follower) {
        if (role_ == role::leader)
            dispatcher_->cancel(heartbeat_timer_id_);
        else if(role_ == role::candidate)
            dispatcher_->cancel(election_timer_id_);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            role_ = role::follower;
            current_term_ = term;
            voted_for_ = -1;
        }
        reschedule_election();
    }
    else if(r == role::candidate) {
        // on conversion to candidate, start election
        {
            std::lock_guard<std::mutex> lock(mutex_);
            role_ = role::candidate;
            ++current_term_;           // increment current_term
            voted_for_ = id_;          // vote for itself
            votes_ = 1;
        }
        send_request_vote_args();  // start request_vote RPCs to all other servers
    }
    else {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            role_ = role::leader;
            // reinitialized after election
            int last_log_index = log_entries_.size();
            // initialized to leader last_log_index + 1
            for(auto& i : next_index_)
                i = last_log_index + 1;        
            for(auto& i : match_index_)
                i = 0;
        }
        // Upon election: send initial empty append_entries RPCs
        // (heartbeat) to each server; repeat during idle periods to
        // prevent election timeouts
        heartbeat_timer_id_ = dispatcher_->run_every(0, HEARTBEAT_INTERVAL, [this]{ this->send_append_entries_args(true); });
    }
}

int raft_server::get_election_timeout() const
{
    // seed with a real random value
    static std::random_device r;
    static std::default_random_engine engine(r());
    // choose a random mean between TIMEOUT_BASE and TIMEOUT_TOP
    static std::uniform_int_distribution<int> uniform_dist(TIMEOUT_BASE, TIMEOUT_TOP);
    return uniform_dist(engine);
}

void raft_server::send_request_vote_args()
{
    std::unique_lock<std::mutex> lock(mutex_);

    request_vote_args args{current_term_, id_, log_entries_.back().index, log_entries_.back().term};

    int size = peers_.size();

    rpc_id_t rpcids[size];

    for(size_t i = 0; i < size; i++)
        rpcids[i] = peers_[i]->call("raft::request_vote", args, &raft_server::receive_request_vote_reply);     

    election_timer_id_ = dispatcher_->run_after(get_election_timeout(), [this, size, =] {
        for(size_t i = 0; i < size; i++)
            peers_[i]->cancel(rpcids[i]);

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (role_ == role::candidate)
                send_request_vote_args();
        }
    });
}

void raft_server::request_vote(const request_vote_args& args, request_vote_reply& reply)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (args.term < current_term_) {
        reply.term = args.term;
        reply.vote_granted = false;
        return;
    }
    else if (args.term > current_term_)
        convert_to(role::follower, args.term);
    
    if (voted_for_ == -1 || voted_for_ == args.candidate_id) {
        if (log_entries_.back().index == args.last_log_index && log_entries_.back().term == args.last_log_term) {
            reply.term == args.term;
            reply.vote_granted = true;
            voted_for_ = args.candidate_id;
            return;
        } 
    }

    reply.term = args.term;
    reply.vote_granted = false;
}

void raft_server::receive_request_vote_reply(int id, const request_vote_reply& reply)
{
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (role_ != role::candidate)
        return;
    
    if (reply.term > current_term_)
        convert_to(role::follower, reply.term);

    // if votes received from mojority of servers: become leader
    if (reply.term == current_term_ && reply.vote_granted)
        if (++votes_ >= this->majority())
            convert_to(role::leader);
}

void raft_server::send_append_entries_args(bool heartbeat)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (role_ != role::leader)
        return;
       
    int size = peers_.size();
    append_entries_args args[size];

    rpc_id_t rpcids[size];
    
    for(int i = 0; i < size; i++)
    {
        std::vector<struct log> entries;
        for(int j = next_index_[i]; j < log_entries_.size(); j++)    
            entries.push_back(log_entries_[j]);

        args[i] = {current_term_, id_, log_entries_[next_index_[i]-1].index, log_entries_[next_index_[i]-1].term, commit_index_, entries};

        rpcids[i] = peers_[i]->call("raft::append_entries", args[i], i, raft_server::receive_append_entries_reply);
    }

    timer_id_ = dispatcher_->run_after(TIMEOUT_TOP, [this, size]{
        for(int i = 0; i < size; i++)
            peers_[i]->cancel(rpcids[i]);
    }); 
}

void raft_server::append_entries(const append_entries_args& args, append_entries_reply& reply)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // reply false if term < current_term
    if (args.term < current_term_) {
        reply.term = args.term;
        reply.success = false;
        return;
    }

    // reply false if log doesn't contain an entry 
    // at prev_log_index whose term matches prev_log_term
    if (log_entries_.size() < args.prev_log_index) {
        reply.term = args.term;
        reply.success = false;
        return;
    }
    
    if (log_entries_[args.prev_log_index].term != args.prev_log_term) {
        reply.term = args.term;
        reply.success = false;
        return;
    }

    // if an existing entry conflicits with a new one 
    // (same index but different terms),
    // delete the existing entry and all that follow it
    int index;
    for(auto& log : args.log_entries)
    {
        index = log.index;
        int term = log.term;
        if (log_entries_[index].term != term) {
            log_entries_.erase(log_entries_.begin()+index, log_entries_.end());
            break;
        }    
    }
    
    // append any new entries not already in the log
    auto it = args.log_entries.begin()+index;
    for(; it != args.log_entries.end(); it++)
        log_entries_.push_back(*it);
    
    // if leader_commit > commit_index
    // set commit_index = min(leader_commit, index_of_last new entry)
    if (args.leader_commit > commit_index_)
        commit_index_ = std::min(args.leader_commit, uint64_t(log_entries_.size()-1));
}

void raft_server::receive_append_entries_reply(int id, const append_entries_reply& reply)
{
    // determine if it is leader
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (role_ != role::leader)
        return;
    
    // if append_entries RPC received from new leader: convert to follower
    if (reply.term > current_term_)
        convert_to(role::follower, reply.term);
    
    // If successful: update next_index and match_index for follower
    if (reply.success) {
        next_index_[id]++;
        match_index_[id]++;
    }
    else {
        // If append_entries fails because of log inconsistency: 
        // decrement next_index and retry
        next_index_[id]--;
        send_append_entries_args(false);
    }

    update_commit_index();
}

void raft_server::add_peer(rpc_conn* conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    peers_.push_back(conn);
    next_index_.push_back(0);
    match_index_.push_back(0);
}

// If election timeout elapses without receiving append_entries
// RPC from current leader or granting vote to candidate:
// convert to candidate
void raft_server::reschedule_election()
{
    // If election timeout elapses : start new election
    election_timer_id_ = dispatcher_->run_after(get_election_timeout(), [this]{ send_request_vote_args(); }); 
}

void raft_server::update_commit_index()
{
    // if there exists an N such that N > commit_index_
    int n = commit_index_ + 1;
    
    for(; n < log_entries_.size(); n++)
    {
        int count = 0;

        for(int i = 0; i < peers_.size; i++)
            if (match_index_[i] >= n)
                count++;

        // a majority of match_index_[i] >= N, and log[N].term == current_term_
        // set commit_index = N;
        if (count >= majority() && log_entries_[n].term == current_term_)
            commit_index_ = n;
    }
}

void raft_server::read_state_from_disk()
{
    static std::string filename = "raft_server_state.data";
    static std::fstream fs(filename, fs.binary | fs.in);

    fs.seekg(0);

    int current_term, voted_for, size;
    std::vector<struct log> log_entries;

    fs >> current_term >> voted_for >> size;
    
    for(int i = 0; i < size; i++)
    {
        struct log entry;
        fs >> entry;
        log_entries.push_back(entry);
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_term_ = current_term;
        voted_for_ = voted_for;
        log_entries_ = log_entries;
    }
}

void raft_server::persist_state_to_disk()
{
    static std::string filename = "raft_server_state.data";
    static std::fstream fs(filename, fs.binary | fs.trunc | fs.out);

    int current_term, voted_for;
    std::vector<struct log> log_entries;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        current_term = current_term_;
        voted_for = voted_for_;
        log_entries = log_entries_;
    }

    fs.seekp(0);

    fs << current_term_ << voted_for << log_entries.size();
    
    for(auto& log : log_entries)
        fs << log;
}

bool raft_server::apply_to_statemachine()
{
    bool ret;
    if (commit_index_ > last_applied_) {
        ++last_applied_;
        ret = apply_func_(log_entries_[last_applied_]);
        if (!ret)
            --last_applied_;
    }
    return ret;
}

void raft_server::set_apply_func(std::function<bool(struct log)> func)
{
    apply_func_ = func;
}

std::ostream& operator<<(std::ostream& os, const raft_server& server)
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    bool flag = (server.role_ == role::leader);
    os << "leader:" << flag 
       << "term: " << server.current_term_
       << "commit: " << server.commit_index_
       << "size: " << server.log_entries_.size() << std::endl;
    return os;
}