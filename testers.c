#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "testers.h"
#include "monitor.h"

#define check_error(cnd, msg)\
    do{\
        if (!(cnd)){\
            if (errno != 0) perror(msg);\
            else fprintf(stderr, "Error: %s\n", (msg));\
            exit(EXIT_FAILURE);\
        }\
    } while(0)

#define pthread_check_error(func, msg)\
    do{\
        int ret = func;\
        if (ret != 0){\
            errno = ret;\
            check_error(0, (msg));\
        }\
    } while(0)

#define MAX_ATTEMPTS 128

struct monitor m;
int monitor_global_counter;

void* monitor_inc(void* arg){
    monitor_global_counter++;
    return NULL;
}

void* inc_tf(void* arg){
    sleep(1);
    struct mreturn ret = m_call(&m, monitor_inc, NULL);
    check_error(ret.a == 0, "m_call");

    return NULL;
}

int incrementer_test(int n){
    check_error(m_init(&m, 0) == 0, "m_init");    
    monitor_global_counter = 0;

    pthread_t* threads = calloc(n, sizeof(threads[0]));
    check_error(threads != NULL, "calloc");

    for (int i = 0; i < n; i++){
        int repeat = 0;
        int attempts = 0;
        do {
            repeat = pthread_create(&(threads[i]), NULL, inc_tf, NULL);
            if (repeat != 0 && (repeat != EAGAIN || attempts > MAX_ATTEMPTS)){
                errno = repeat;
                check_error(0, "pthread_create");
            }
            else if (repeat != 0){
                sleep(1);
                attempts++;
            }
        } while(repeat);
    }

    for (int i = 0; i < n; i++){
        pthread_check_error(pthread_join(threads[i], NULL), "pthread_join");
    }

    check_error(m_delete(&m) == 0, "m_delete");

    return monitor_global_counter;
}
