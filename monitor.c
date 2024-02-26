#include "monitor.h"
#include <semaphore.h>
#include <stdarg.h>
#include <stdlib.h>

int m_init(struct monitor* m, size_t n, ...){
    va_list ap; 
    va_start(ap, n);

    m->n_cond = n;
    m->cond = NULL;
    if (m->n_cond > 0){
        m->cond = calloc(m->n_cond, sizeof(m->cond[0]));
        if (m->cond == NULL) return -1;

        for (size_t i = 0; i < n; i++){
            m->cond[i] = va_arg(ap, conditional_t*);
            int _ = sem_init(&(m->cond[i]->s), 0, 0);
            if (_ != 0){
                for (size_t j = 0; j < i; j++)
                    sem_destroy(&(m->cond[j]->s));
                free(m->cond);
                va_end(ap);
                return -1;
            }
            m->cond[i]->n = 0;
        }
    }

    int a_ = sem_init(&(m->mutex), 0, 1);
    int b_ = sem_init(&(m->sleep), 0, 0);

    if (a_ != 0 || b_ != 0){
        for (size_t j = 0; j < n; j++)
            sem_destroy(&(m->cond[j]->s));
        free(m->cond);
        if (a_ == 0) sem_destroy(&(m->mutex));
        if (b_ == 0) sem_destroy(&(m->sleep));
        va_end(ap);
        return -1;
    }

    m->n_sleeping = 0;

    va_end(ap);
    return 0;
}

