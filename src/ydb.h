// This file defines the public interface to the ydb library

#if !defined(TOKU_YDB_INTERFACE_H)
#define TOKU_YDB_INTERFACE_H

#if defined(__cplusplus)
extern "C" {
#endif

// Initialize the ydb library globals.  
// Called when the ydb library is loaded.
int toku_ydb_init(void);

// Called when the ydb library is unloaded.
int toku_ydb_destroy(void);

// Called to use dlmalloc functions.
void setup_dlmalloc(void) __attribute__((__visibility__("default")));

// db_env_create for the trace library
int db_env_create_toku10(DB_ENV **, u_int32_t) __attribute__((__visibility__("default")));

// db_create for the trace library
int db_create_toku10(DB **, DB_ENV *, u_int32_t) __attribute__((__visibility__("default")));

// test only function
int toku_test_db_redirect_dictionary(DB * db, char * dname_of_new_file, DB_TXN *dbtxn) __attribute__((__visibility__("default")));

uint64_t toku_test_get_latest_lsn(DB_ENV *env) __attribute__((__visibility__("default")));

// test-only function
extern int toku_test_get_checkpointing_user_data_status(void) __attribute__((__visibility__("default")));

void toku_test_set_yield_callback(void (*yield)(void *extra), void *extra) __attribute__((__visibility__("default")));

#if defined(__cplusplus)
}
#endif

#endif
