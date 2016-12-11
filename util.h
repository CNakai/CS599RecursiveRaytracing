#ifdef DEBUG
    void secret_DEBUG_LOG(char *);
    #define DEBUG_LOG(msg) secret_DEBUG_LOG(msg)
#else
    #define DEBUG_LOG(ignore) ((void) 0)
#endif


void* checked_malloc(size_t);
void report_error_and_exit(char*);
