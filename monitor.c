#include "monitor.h"
#include <stdlib.h>
#include <stdio.h>

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

int m_delete(struct monitor* m){
    int r = 0;
    for (size_t i = 0; i < m->n_cond; i++){
        if (sem_destroy(&(m->cond[i]->s)) != 0) r = -1;
    }
    free(m->cond);
    if (sem_destroy(&(m->mutex)) != 0) r = -1;
    if (sem_destroy(&(m->sleep)) != 0) r = -1;
    return r;
}

struct mreturn m_call(struct monitor* m, void* (*f)(void*), void* args){
    struct mreturn r;
    r.a = 0;
    r.ret = NULL;
    
    if (sem_wait(&(m->mutex)) != 0){
        r.a = -1;
        return r;
    }

    r.ret = f(args);

    if (m->n_sleeping > 0){
        if (sem_post(&(m->sleep)) != 0){
            sem_post(&(m->mutex));
            r.a = -1;
            return r;
        }
        m->n_sleeping--;
    }
    else{
        if (sem_post(&(m->mutex)) != 0){
            r.a = -1;
            return r;
        }
    }

    return r;
}

int mmutex_sleep(struct monitor* m){
    int r = 0;
    size_t oldsleeping = m->n_sleeping;
    m->n_sleeping++;
    if (oldsleeping > 0){
        if (sem_post(&(m->sleep)) != 0){
            r -= -1;
        }
    }
    if (oldsleeping <= 0 || r == -1){
        if (sem_post(&(m->mutex)) != 0){
            r -= -1;
        }
    }
    if (r == -2){
        perror("mmutex_sleep failed to open either semaphore");
        exit(EXIT_FAILURE);
    }
    if (sem_wait(&(m->sleep)) != 0){
        r = -1;
    }
    m->n_sleeping--;
    return r;
}

int mmutex_wait(struct monitor* m, conditional_t* cv){
    int r = 0;
    size_t oldsleeping = m->n_sleeping;
    cv->n++;
    
    if (oldsleeping > 0){
        if (sem_post(&(m->sleep)) != 0){
            r -= -1;
        }
    }
    if (oldsleeping <= 0 || r == -1){
        if (sem_post(&(m->mutex)) != 0){
            r -= -1;
        }
    }

    if (r == -2){
        perror("mmutex_wait failed to open either semaphore");
        exit(EXIT_FAILURE);
    }
    if (sem_wait(&(cv->s)) != 0){
        r = -1;
    }
    cv->n--;
    return r;
}

int mmutex_signal(struct monitor* m, conditional_t* cv){
    int r = 0;
    if (cv->n > 0){
        m->n_sleeping++;
        if (sem_post(&(cv->s)) != 0){
            r = -1;
        }
        if (r == 0){
            if (sem_wait(&(m->sleep)) != 0){
                r = -1;
            }
        }
        m->n_sleeping--;
    }
    return r;
}

