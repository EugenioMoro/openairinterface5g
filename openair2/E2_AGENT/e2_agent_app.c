//
// Created by root on 6/30/22.
//

#include "e2_agent_app.h"
#include "flexran_agent.h"
#include <common/utils/system.h>

#include <pthread.h>
#include <arpa/inet.h>

#include "common/ran_context.h"
#include "common/utils/LOG/log.h"

#include "pb_encode.h"
#include "pb_decode.h"

#include "proto/e2.pb.h"
#include "E2_requests.h"
#include "e2_prtbf_common.h"

extern RAN_CONTEXT_t RC;

int agent_task_created = 0;
pthread_t heartbeat_thread; // heartbeat has a mutex to easily stop heartbeat messages if needed (just lock the mutex)

int e2_agent_init(){
    e2_agent_info_t* agent_info = malloc(sizeof(e2_agent_info_t));
    LOG_I(E2_AGENT,"Initializing E2 agent\n");


    // heartbeat thread and mutex init
    if(pthread_create(&heartbeat_thread,NULL,&e2_heartbeat, agent_info)){
        LOG_E(E2_AGENT,"Could not init heartbeat thread\n");
        return -1;
    }
    pthread_detach(heartbeat_thread);

    if (pthread_mutex_init(&(agent_info->hb_mutex), NULL) != 0)
    {
        LOG_E(E2_AGENT,"Could not init mutex\n");
        return -1;
    }

    // network init
    agent_info->listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(agent_info->listenfd, SOL_SOCKET, SO_REUSEADDR, &(agent_info->reuse), sizeof(agent_info->reuse));

    memset(&(agent_info->servaddr), 0, sizeof(agent_info->servaddr));
    agent_info->servaddr.sin_family = AF_INET;
    agent_info->servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    agent_info->servaddr.sin_port = htons(1234);
    if (bind(agent_info->listenfd, (struct sockaddr *) &(agent_info->servaddr), sizeof(agent_info->servaddr)) != 0) {
        LOG_E(E2_AGENT, "Socket binding error\n");
        return -1;
    }

    if (listen(agent_info->listenfd, 5) != 0) {
        LOG_E(E2_AGENT, "Socket listening error\n");
        return -1;
    }

    // create itti task
    if(itti_create_task(TASK_E2_AGENT,e2_agent_task, agent_info) < 0){
        LOG_E(E2_AGENT, "cannot create ITTI task\n");
        return -1;
    }
    return 0;
}

void* e2_heartbeat(void* args) {
    // this is mutex protected such that heartbeats can be stopped anytime by locking the mutex sw else
    INFINITE_LOOP{
        pthread_mutex_lock(&((e2_agent_info_t*) args)->hb_mutex);
        LOG_I(E2_AGENT, "E2 agent heartbeat\n");
        pthread_mutex_unlock(&((e2_agent_info_t*) args)->hb_mutex);
        sleep(3);
    }
}

void *e2_agent_task(void* args_p){
    e2_agent_info_t* e2_info = args_p;
    itti_mark_task_ready(TASK_E2_AGENT);
    INFINITE_LOOP {
        /* Wait for a client */
        e2_info->connfd = accept(e2_info->listenfd, NULL, NULL);

        if (e2_info->connfd < 0) {
            LOG_E(E2_AGENT, "Socket accept error\n");
            continue;
        }
        LOG_I(E2_AGENT, "Got connection\n");

        handle_connection(e2_info->connfd);

        printf("Closing connection.\n");

        close(e2_info->connfd);
    }

}


void build_dummy_response(E2_REQID_t req_id, E2_dummy_response *rsp, int connfd) {
    rsp->req_id = req_id;
    strcpy(rsp->mess_string,"Hi, this is a dummy response!!");
    rsp->result = true;
}

void build_send_dummy_response(E2_REQID_t req_id, E2_dummy_response* rsp, int connfd){
    rsp->req_id = 100;
    rsp->req_id = req_id;
    strcpy(rsp->mess_string,"Hi, this is a dummy response!!");
    rsp->result = true;
    pb_ostream_t output = pb_ostream_from_socket(connfd);
    if (!pb_encode_delimited(&output, E2_dummy_response_fields, rsp)) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&output));
    }
    printf("Encoded in %lu bytes\n", output.bytes_written);
}

void ship_response(pb_msgdesc_t msg_descriptor, void* response, int connfd){
    pb_ostream_t output = pb_ostream_from_socket(connfd);
    if (!pb_encode_delimited(&output, &msg_descriptor, response)) {
        printf("Encoding failed: %s\n", PB_GET_ERROR(&output));
    }
    // printf("Encoded in %lu bytes\n", output.bytes_written);
}

void handle_connection(int connfd) {

    E2_request request = {};
    pb_istream_t input = pb_istream_from_socket(connfd);
    if (!pb_decode_delimited(&input, E2_request_fields, &request)) {
        printf("Decode failed: %s\n", PB_GET_ERROR(&input));
        return;
    }
    printf("Received E2 request id %d\n", request.req_id);
    switch (request.req_id++) {
        case E2_REQID_DUMMY1:{
            E2_dummy_response* rsp = malloc(E2_dummy_response_size);
            build_dummy_response(E2_REQID_DUMMY1,rsp, connfd);
            ship_response(E2_dummy_response_msg,rsp,connfd);
            free(rsp);
            break;
        }
        case E2_REQID_DUMMY2: {
            E2_dummy_response *rsp = malloc(E2_dummy_response_size);
            build_dummy_response(E2_REQID_DUMMY2, rsp, connfd);
            ship_response(E2_dummy_response_msg, rsp, connfd);
            free(rsp);
            break;
        }
        default:
            printf("Unrecognized request id %d\n",request.req_id);
            break;
    }
}