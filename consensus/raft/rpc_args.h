#pragma once

#include "log.h"
#include "codec.h"
#include "coding.h"

#include <vector>
#include <cstdint>

// Invoked by candidates to gather votes
struct request_vote_args
{
    uint64_t term;           // candidate's term
    uint32_t candidate_id;   // candidate requesting vote
    uint64_t last_log_index; // index of candidate's last log entry
    uint64_t last_log_term;  // term of candidate's last log entry
};

struct request_vote_reply
{
    uint64_t term;           // current term, for candidate to update itself
    uint8_t  vote_granted;   // true means candidate received vote
};

// Invoked by leader to replicate log entries; also used as heartbeat
struct append_entries_args
{
    uint64_t term;                       // leader's term
    uint8_t  leader_id;                  // so follower can redirect clients
    uint64_t prev_log_index;             // index of log entry immediately preceding new onces
    uint64_t prev_log_term;              // term of prev_log_index entry
    uint64_t leader_commit;              // leader's commit_index
    std::vector<struct log> log_entries; // log entries to store(empty for heartbeat; may send more than one for efficiency)
};

struct append_entries_reply
{
    uint64_t term;
    uint8_t  success;
};