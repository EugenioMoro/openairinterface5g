//
// Created by root on 6/30/22.
//

#ifndef OPENAIRINTERFACE_E2_AGENT_APP_H
#define OPENAIRINTERFACE_E2_AGENT_APP_H

#endif //OPENAIRINTERFACE_E2_AGENT_APP_H

#ifndef INFINITE_LOOP
#define INFINITE_LOOP for(;;)
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct e2_agent_info_t{
    int status;
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    int reuse;
    pthread_mutex_t hb_mutex;
} e2_agent_info_t;

int e2_agent_init();

void *e2_agent_task();

void *e2_heartbeat();