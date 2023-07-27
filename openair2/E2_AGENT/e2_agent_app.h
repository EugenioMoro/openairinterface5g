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

#include "oai-oran-protolib/builds/ran_messages.pb-c.h"

#define E2AGENT_IN_PORT 6655
#define E2AGENT_OUT_PORT 6600
#define E2AGENT_MAX_BUF_SIZE 4096

typedef struct e2_agent_info_t{
    int status;
    int in_sockfd, out_sockfd;
    struct sockaddr_in out_sockaddr,in_sockaddr;
    int reuse;
    pthread_mutex_t hb_mutex;
} e2_agent_info_t;

/*
this struct will be globally accessible by anyone and it contains values coming from the xapp 
to be asynchr. read by other parts of the code
*/
typedef struct e2_agent_databank{
    int max_prb;
    int true_gbr;
    pthread_mutex_t mutex;
} e2_agent_databank_t;

extern e2_agent_databank_t* e2_agent_db;

int e2_agent_init();

void *e2_agent_task();

void *e2_heartbeat();

void handle_master_message(void* buf, int buflen, int out_socket, struct sockaddr_in servaddr);