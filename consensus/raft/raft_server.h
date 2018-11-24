#pragma once

#include "log.h"
#include "rpc_args.h"
#include "rpc_server.h"
#include "rpc_conn.h"
#include "dispatcher.h"

#include <mutex>
#include <vector>
#include <iostream>

const int TIMEOUT_BASE{150};
const int TIMEOUT_TOP{300};
const int ELECTION_TIMEOUT_BASE{300};
const int HEARTBEAT_INTERVAL{30};

enum class role {
    follower,
    candidate,
    leader
};

class raft_server: public rpc_server
{
private:
    timer_id_t timer_id_;
    timer_id_t election_timer_id_;
    timer_id_t heartbeat_timer_id_;
    
    std::mutex mutex_;

    int id_;
    role role_;
    int votes_;
    // persistent state on all servers(updated on stable storage before responding to RPCs)

    // last term server has seen (initialized to 0 on first boot, increases monotonically)
    uint64_t current_term_;
    // candidate_id that received vote in current term (or null if none)          
    int voted_for_;                      
    // log entries; each entry contains command for state machine, 
    // and term when entry was received by leader(first index is 1)  
    std::vector<struct log> log_entries_;

    // volatile state on all servers

    // index of highest log entry known to be commited(initialized to 0, increases monotonically)
    uint64_t commit_index_;   
    // index of highest log entry applied to state machine(initialized to 0, increases monotonically)        
    uint64_t last_applied_;               

    // volatile state on leaders(reinitialized after election)
    
    // for each server, index of the next log entry 
    // to send to that server(initialized to leader last log index + 1)
    std::vector<int> next_index_;    
    // for each server, index of highest log entry 
    // known to be replicated on server(initialized to 0, increases monotonically)
    std::vector<int> match_index_;   

    std::vector<rpc_conn *> peers_;
    std::function<bool(struct log)> apply_func_;
public:
    raft_server(dispatcher* d, short port, int id);
    ~raft_server();

    friend std::ostream& operator<<(std::ostream& os, const raft_server& server);

    void start() override;
    void shutdown() override;

    bool replicate(struct log entry);

    void add_peer(rpc_conn* conn);
    /*
        +----1. candidaete call send_request_vote_args();
        rpc  2. follower call method request_vote();
        +----3. candidate call receive_request_vote_reply();

            candidate  (marshal)   1.-----<request_vote_args>------->2.-(rpc)-+  (unmarshall) follower
            candidate (unmarshall) 3.<----<request_vote_reply>-------2.-(rpc)-+   (marshall)  follower
                |
  follower <----+---->leader
    */
    void request_vote(const request_vote_args& args, request_vote_reply& reply);

    /*
        +----1. leader call send_append_entries_args();
        rpc  2. follower call method append_entries();
        +----3. leader call receive_append_entries_reply();

             leader  (marshal)   1.-----<append_entries_args>----->2.-(rpc)-+ (unmarshal) follower
             leader (unmarshall) 3.<----<append_entries_reply>-----2.-(rpc)-+  (marshal)  follower
                |
  follower <----+---->leader
    */
    void append_entries(const append_entries_args& args, append_entries_reply& reply);

    void set_apply_func(std::function<bool(struct log)> func);
private:
    int majority() const;
    bool is_leader(int term) const;
    int get_election_timeout() const;

    void convert_to(role r, int term = -1);
    
    // candidate action
    void send_request_vote_args();
    // candidate action
    void receive_request_vote_reply(int id, const request_vote_reply& reply);

    // leader action
    void send_append_entries_args(bool heartbeat);
    // leader action
    void receive_append_entries_reply(int id, const append_entries_reply& reply);

    void reschedule_election();

    void update_commit_index();

    // updated on stable storage before responding to RPCs
    void read_state_from_disk();
    void persist_state_to_disk();

    bool apply_to_statemachine();
};