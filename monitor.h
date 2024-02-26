#pragma once
#include <semaphore.h>
#include <stdarg.h>
#include <aio.h>

typedef struct{
    sem_t s;        // semaphore for waiting
    size_t n;       // number of waiting processes
} conditional_t;

struct monitor{
    conditional_t **cond;   // array of pointers to conditional_t
    size_t n_cond;          // length of cond 
    sem_t mutex;            // mutex protecting monitor
    sem_t sleep;            // internal semaphore for temporarily waiting processes
    size_t n_sleeping;      // number of such processes
};

struct mreturn{
    int a;          // 0 on success, -1 on failure
    void* ret;      // return of function pointer
};

// ... is n pointers to conditional_t
int m_init(struct monitor* m, size_t n, ...);

struct mreturn m_call(struct monitor* m, void* (*f)(void*), void* args);

// the following functions assume current thread has locked m->mutex
int mmutex_sleep(struct monitor* m);
int mmutex_wait(struct monitor* m, conditional_t* cv);
int mmutex_signal(struct monitor* m, conditional_t* cv);

