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
#include "e2_message_handlers.h"


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
    // create sockets
    if((agent_info->in_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to create in socket\n");
        exit(EXIT_FAILURE);
    }
    setsockopt(agent_info->in_sockfd, SOL_SOCKET, SO_REUSEADDR, &(agent_info->reuse), sizeof(agent_info->reuse));

    if((agent_info->out_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to create out socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&(agent_info->out_sockaddr), 0, sizeof(agent_info->out_sockaddr));
    memset(&(agent_info->in_sockaddr), 0, sizeof(agent_info->in_sockaddr));

    agent_info->out_sockaddr.sin_family = AF_INET;
    agent_info->out_sockaddr.sin_addr.s_addr = INADDR_ANY;
    agent_info->out_sockaddr.sin_port = htons(E2AGENT_OUT_PORT);

    agent_info->in_sockaddr.sin_family = AF_INET;
    agent_info->in_sockaddr.sin_addr.s_addr = INADDR_ANY;
    agent_info->in_sockaddr.sin_port = htons(E2AGENT_IN_PORT);

    if (bind(agent_info->in_sockfd, (struct sockaddr *) &(agent_info->in_sockaddr), sizeof(agent_info->in_sockaddr)) != 0) {
        perror("Failed to bind in socket");
        exit(EXIT_FAILURE);
    }
    LOG_E(E2_AGENT, "Agent waiting for UDP datagrams\n");

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
    uint8_t recv_buf[E2AGENT_MAX_BUF_SIZE];
    int rcv_len;
    unsigned slen;
    slen = sizeof(e2_info->in_sockaddr);
    itti_mark_task_ready(TASK_E2_AGENT);
    INFINITE_LOOP {
        /* Wait for a client */
        rcv_len = recvfrom(e2_info->in_sockfd, recv_buf, E2AGENT_MAX_BUF_SIZE, 0, (struct sockaddr *) &(e2_info->in_sockaddr), &slen);
        LOG_I(E2_AGENT, "Received %d bytes\n", rcv_len);
        handle_master_message(recv_buf, rcv_len, e2_info->out_sockfd, e2_info->out_sockaddr);
    }

}
