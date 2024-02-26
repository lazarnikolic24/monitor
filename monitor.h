#pragma once
#include <semaphore.h>

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

// ... is n pointers to conditional_t
int m_init(struct monitor* m, size_t n, ...);



