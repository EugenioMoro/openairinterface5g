//
// Created by root on 10/28/22.
//

#ifndef OPENAIRINTERFACE_E2_MESSAGE_HANDLERS_H
#define OPENAIRINTERFACE_E2_MESSAGE_HANDLERS_H

#endif //OPENAIRINTERFACE_E2_MESSAGE_HANDLERS_H

#include "oai-oran-protolib/builds/ran_messages.pb-c.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#include "NR_MAC_gNB/nr_mac_gNB.h"

typedef struct sockaddr_in sockaddr_in;

void handle_subscription(RANMessage* in_mess);
void handle_indication_request(RANMessage* in_mess,int out_socket, sockaddr_in servaddr);
void handle_indication_response(RANMessage* in_mess, int out_socket, sockaddr_in servaddr);
void handle_control(RANMessage* in_mess);
const char* get_enum_name(RANParameter ran_par_enum);
void ran_write(RANParamMapEntry* target_param_map_entry);
char* ran_read(RANParameter ran_par_enum);
inline char* mymy_itoa(int i);
void handle_master_message(void* buf, int buflen, int out_socket, struct sockaddr_in servaddr);
UeListM* get_ue_list();
void ran_read_new(RANParameter ran_par_enum, RANParamMapEntry* map_entry);
float_t get_nr_RSRP(module_id_t Mod_id,uint8_t CC_id,uint8_t gNB_index);