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

#include "pb_encode.h"
#include "pb_decode.h"

#include "proto/e2.pb.h"
#include "E2_requests.h"
#include "e2_prtbf_common.h"

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

void build_dummy_response(E2_REQID_t req_id, E2_dummy_response* rsp, int connfd);
void build_send_dummy_response(E2_REQID_t req_id, E2_dummy_response* rsp, int connfd);
void ship_response(pb_msgdesc_t msg_descriptor, void* response, int connfd);
void handle_connection(int connfd);