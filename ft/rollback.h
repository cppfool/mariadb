/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ifndef TOKU_ROLLBACK_H
#define TOKU_ROLLBACK_H

#ident "$Id$"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."
#ident "The technology is licensed by the Massachusetts Institute of Technology, Rutgers State University of New Jersey, and the Research Foundation of State University of New York at Stony Brook under United States of America Serial No. 11/760379 and to the patents and/or patent applications resulting from it."

#include "omt.h"
#include "sub_block.h"

void toku_poll_txn_progress_function(TOKUTXN txn, uint8_t is_commit, uint8_t stall_for_checkpoint);

// these functions assert internally that they succeed

// get a rollback node this txn may use for a new entry. if there
// is a current rollback node to use, pin it, otherwise create one.
void toku_get_and_pin_rollback_log_for_new_entry(TOKUTXN txn, ROLLBACK_LOG_NODE *log);

// get a specific rollback by blocknum and hash
void toku_get_and_pin_rollback_log(TOKUTXN txn, BLOCKNUM blocknum, uint32_t hash, ROLLBACK_LOG_NODE *log);

// unpin a rollback node from the cachetable
void toku_rollback_log_unpin(TOKUTXN txn, ROLLBACK_LOG_NODE log);

// assert that the given log's txnid and sequence match the ones given
void toku_rollback_verify_contents(ROLLBACK_LOG_NODE log, TXNID txnid, uint64_t sequence);

// if there is a previous rollback log for the given log node, prefetch it
void toku_maybe_prefetch_previous_rollback_log(TOKUTXN txn, ROLLBACK_LOG_NODE log);

// unpin and rmove a rollback log from the cachetable
void toku_rollback_log_unpin_and_remove(TOKUTXN txn, ROLLBACK_LOG_NODE log);

void *toku_malloc_in_rollback(ROLLBACK_LOG_NODE log, size_t size);
void *toku_memdup_in_rollback(ROLLBACK_LOG_NODE log, const void *v, size_t len);

// given a transaction and a log node, and if the log is too full,
// set the current rollback log to ROLLBACK_NONE and move the current
// node onto the tail of the rollback node chain. further insertions
// into the rollback log for this transaction will force the creation
// of a new rollback log.
//
// this never unpins the rollback log if a spill occurs. the caller
// is responsible for ensuring the given rollback node is unpinned
// if necessary.
void toku_maybe_spill_rollbacks(TOKUTXN txn, ROLLBACK_LOG_NODE log);

void toku_txn_maybe_note_ft (TOKUTXN txn, FT h);
int toku_logger_txn_rollback_raw_count(TOKUTXN txn, uint64_t *raw_count);

int toku_find_xid_by_xid (const TXNID &xid, const TXNID &xidfind);

PAIR_ATTR rollback_memory_size(ROLLBACK_LOG_NODE log);

// A high-level rollback log is made up of a chain of rollback log nodes.
// Each rollback log node is represented (separately) in the cachetable by 
// this structure. Each portion of the rollback log chain has a block num
// and a hash to identify it.
struct rollback_log_node {
    int                layout_version;
    int                layout_version_original;
    int                layout_version_read_from_disk;
    uint32_t           build_id;      // build_id (svn rev number) of software that wrote this node to disk
    int                dirty;
    // to which transaction does this node belong?
    TXNID              txnid;
    // sequentially, where in the rollback log chain is this node? 
    // the sequence is between 0 and totalnodes-1
    uint64_t           sequence;
    BLOCKNUM           blocknum; // on which block does this node live?
    uint32_t           hash;
    // which block number is the previous in the chain of rollback nodes 
    // that make up this rollback log?
    BLOCKNUM           previous; 
    uint32_t           previous_hash;
    struct roll_entry *oldest_logentry;
    struct roll_entry *newest_logentry;
    MEMARENA           rollentry_arena;
    size_t             rollentry_resident_bytecount; // How many bytes for the rollentries that are stored in main memory.
    PAIR               ct_pair;
};

struct serialized_rollback_log_node {
    char *data;
    uint32_t len;
    int n_sub_blocks;
    BLOCKNUM blocknum;
    struct sub_block sub_block[max_sub_blocks];
};

static inline void
toku_static_serialized_rollback_log_destroy(SERIALIZED_ROLLBACK_LOG_NODE log) {
    toku_free(log->data);
}

static inline void
toku_serialized_rollback_log_destroy(SERIALIZED_ROLLBACK_LOG_NODE log) {
    toku_static_serialized_rollback_log_destroy(log);
    toku_free(log);
}

void rollback_empty_log_init(ROLLBACK_LOG_NODE log);
void make_rollback_log_empty(ROLLBACK_LOG_NODE log);

static inline bool rollback_log_is_unused(ROLLBACK_LOG_NODE log) {
    return (log->txnid == TXNID_NONE);
}


#endif // TOKU_ROLLBACK_H
