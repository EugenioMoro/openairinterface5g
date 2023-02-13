//
// Created by root on 10/28/22.
//

#include "e2_message_handlers.h"
#include "common/ran_context.h"

#include "PHY/defs_nr_UE.h"
#include "PHY/phy_extern_nr_ue.h"
extern RAN_CONTEXT_t RC;
extern PHY_VARS_NR_UE ***PHY_vars_UE_g;

int gnb_id = 0;
int something = 0;
void handle_subscription(RANMessage* in_mess){
    printf("Not implemented\n");
    assert(0!=0);
}
void handle_indication_request(RANMessage* in_mess,int out_socket, sockaddr_in peeraddr){
    printf("Indication request for %lu parameters:\n", in_mess->ran_indication_request->n_target_params);
    for(int par_i=0; par_i<in_mess->ran_indication_request->n_target_params; par_i++){
        printf("\tParameter id %d requested (a.k.a %s)\n",\
        in_mess->ran_indication_request->target_params[par_i],\
        get_enum_name(in_mess->ran_indication_request->target_params[par_i]));
    }
    handle_indication_response(in_mess, out_socket,peeraddr);
}
void handle_indication_response(RANMessage* in_mess, int out_socket, sockaddr_in servaddr){
    // build and fill indication response with target parameters from indication request
    RANIndicationResponse rsp = RAN_INDICATION_RESPONSE__INIT;
    RANParamMapEntry **map;
    void* buf;
    unsigned buflen, i;
    map = malloc(sizeof(RANParamMapEntry*) * in_mess->ran_indication_request->n_target_params);
    for(i=0; i<in_mess->ran_indication_request->n_target_params; i++){
        map[i] = malloc(sizeof(RANParamMapEntry));
        ran_param_map_entry__init(map[i]);
        map[i]->key=in_mess->ran_indication_request->target_params[i];
        ran_read_new(map[i]->key, map[i]);
    }
    rsp.n_param_map=in_mess->ran_indication_request->n_target_params;
    rsp.param_map=map;
    buflen = ran_indication_response__get_packed_size(&rsp);
    buf = malloc(buflen);
    ran_indication_response__pack(&rsp,buf);
    printf("Sending indication response\n");
    unsigned slen = sizeof(servaddr);
    int rev = sendto(out_socket, (const char *)buf, buflen,
                     MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                     slen);
    printf("Sent %d bytes, buflen was %u\n",rev, buflen);
    /*
    printf("Printing buffer for debug pourposes:\n");
    uint8_t* b = (uint8_t*) buf;
    for (int i=0; i<buflen; i++){
        printf(" %hhx ", b[i]);
    }
    printf("\n");
    */
}

void handle_control(RANMessage* in_mess){
    // loop tarhet params and apply
    for(int i=0; i<in_mess->ran_control_request->n_target_param_map; i++){
        printf("Applying target parameter %s with value %s\n",\
        get_enum_name(in_mess->ran_control_request->target_param_map[i]->key),\
        in_mess->ran_control_request->target_param_map[i]->string_value);
        ran_write(in_mess->ran_control_request->target_param_map[i]);
    }
}

const char* get_enum_name(RANParameter ran_par_enum){
    switch (ran_par_enum)
    {
        case RAN_PARAMETER__GNB_ID:
            return "gnb_id";
        case RAN_PARAMETER__SOMETHING:
            return "something";
        case RAN_PARAMETER__UE_LIST:
            return "ue_list";
        default:
            return "unrecognized param";
    }
}

void ran_write(RANParamMapEntry* target_param_map_entry){
    switch (target_param_map_entry->key)
    {
        case RAN_PARAMETER__GNB_ID:
            gnb_id = atoi(target_param_map_entry->string_value);
            break;
        case RAN_PARAMETER__SOMETHING:
            something = atoi(target_param_map_entry->string_value);
            break;
        default:
            printf("ERROR: cannot write RAN, unrecognized target param %d\n", target_param_map_entry->key);
    }
}
/*
char* ran_read(RANParameter ran_par_enum){
    switch (ran_par_enum)
    {
        case RAN_PARAMETER__GNB_ID:
            return my_itoa(gnb_id);
        case RAN_PARAMETER__SOMETHING:
            return my_itoa(something);
        default:
            printf("unrecognized param %d\n",ran_par_enum);
            assert(0!=0);
    }
}
 */

inline char* mymy_itoa(int i){
    int length = (snprintf(NULL, 0,"%d",i)+1);
    char* ret = malloc(length*sizeof(char));
    sprintf(ret, "%d", i);
    return ret;
}

void handle_master_message(void* buf, int buflen, int out_socket, struct sockaddr_in servaddr){
    RANMessage* in_mess = ran_message__unpack(NULL, (size_t)buflen, buf);
    if (!in_mess){
        printf("error decoding received message, printing for debug:\n");
        for(int i=0;i<buflen; i++){
            uint8_t* tempbuf = (uint8_t*) buf;
            printf(" %hhx ", tempbuf[i]);
        }
        printf("\n");
        return;
    }
    printf("ran message id %d\n", in_mess->msg_type);
    switch(in_mess->msg_type){
        case RAN_MESSAGE_TYPE__SUBSCRIPTION:
            printf("Subcription message received\n");
            handle_subscription(in_mess);
            break;
        case RAN_MESSAGE_TYPE__INDICATION_REQUEST:
            printf("Indication request message received\n");
            handle_indication_request(in_mess, out_socket, servaddr);
            break;
        case RAN_MESSAGE_TYPE__INDICATION_RESPONSE:
            printf("Indication response message received\n");
            handle_indication_response(in_mess, out_socket, servaddr);
            break;
        case RAN_MESSAGE_TYPE__CONTROL:
            printf("Control message received\n");
            handle_control(in_mess);
            break;
        default:
            printf("Unrecognized message type\n");
    }
}

// TODO: Update get_ue_list() to work with new gnb data structures
/*
UeListM* get_ue_list(){
    // init ue list
    UeListM* ue_list_m = malloc(sizeof(UeListM));
    ue_list_m__init(ue_list_m);

    NR_UEs_t *UE_info_gnb = &RC.nrmac[0]->UE_info;
    int num_ues = 1;//&RC.nrmac[0]->//UE_info_gnb->;
    //NR_UE_sched_ctrl_t *sched_ctrl = &UE_info->UE_sched_ctrl[UE_id];

    // insert n ues
    ue_list_m->connected_ues = num_ues;
    ue_list_m->n_ue_info = num_ues;
    if(num_ues == 0){
        return ue_list_m;
    }

    // build list of ue_info_m
    UeInfoM** ue_info_list;
    ue_info_list = malloc(sizeof(UeInfoM)*num_ues);
    for(int i = 0; i<num_ues; i++){
        // init list
        ue_info_list[i] = malloc(sizeof(UeInfoM));
        ue_info_m__init(ue_info_list[i]);

        // recover gnb info
        NR_UE_sched_ctrl_t *sched_ctrl = &UE_info_gnb->UE_sched_ctrl[i];
        NR_mac_stats_t *mac_stats = &UE_info_gnb->mac_stats[i];

        ue_info_list[i]->rnti=UE_info_gnb->rnti[i];
        ue_info_list[i]->dlsch_errors=mac_stats->dlsch_errors;
        ue_info_list[i]->dlsch_total_bytes=mac_stats->dlsch_total_bytes;
        ue_info_list[i]->dlsch_current_bytes=mac_stats->dlsch_current_bytes;
        ue_info_list[i]->ulsch_errors=mac_stats->ulsch_errors;
        ue_info_list[i]->ulsch_total_bytes_rx=mac_stats->ulsch_total_bytes_rx;
        ue_info_list[i]->num_rsrp_meas=mac_stats->num_rsrp_meas;
        ue_info_list[i]->sched_ul_bytes=sched_ctrl->sched_ul_bytes;
        ue_info_list[i]->estimated_ul_buffer=sched_ctrl->estimated_ul_buffer;
        ue_info_list[i]->num_total_bytes=sched_ctrl->num_total_bytes;
        ue_info_list[i]->raw_rssi=sched_ctrl->raw_rssi;
        ue_info_list[i]->pusch_snrx10=sched_ctrl->pusch_snrx10;
        ue_info_list[i]->pucch_snrx10=sched_ctrl->pucch_snrx10;
        ue_info_list[i]->ul_rssi=sched_ctrl->ul_rssi;
        ue_info_list[i]->rsrp = 0; // get_nr_RSRP(0,0,0) - 17; // 17 is an offset found somewhere in the code
    }
    // assgin ue info pointer
    ue_list_m->ue_info = ue_info_list;

    return ue_list_m;
}
*/

UeListM* get_ue_list(){
    // init ue list
    UeListM* ue_list_m = malloc(sizeof(UeListM));
    ue_list_m__init(ue_list_m);

    // insert n ues
    ue_list_m->connected_ues = 2;
    ue_list_m->n_ue_info = 2;

    // build list of ue_info_m
    UeInfoM** ue_info_list;
    ue_info_list = malloc(sizeof(UeInfoM)*2);
    for(int i = 0; i<2; i++){
        ue_info_list[i] = malloc(sizeof(UeInfoM));
        ue_info_m__init(ue_info_list[i]);
        ue_info_list[i]->rnti=i;
        ue_info_list[i]->dlsch_errors=i;
        ue_info_list[i]->dlsch_total_bytes=i;
        ue_info_list[i]->dlsch_current_bytes=i;
        ue_info_list[i]->ulsch_errors=i;
        ue_info_list[i]->ulsch_total_bytes_rx=i;
        ue_info_list[i]->num_rsrp_meas=i;
        ue_info_list[i]->sched_ul_bytes=i;
        ue_info_list[i]->estimated_ul_buffer=i;
        ue_info_list[i]->num_total_bytes=i;
        ue_info_list[i]->raw_rssi=i;
        ue_info_list[i]->pusch_snrx10=i;
        ue_info_list[i]->pucch_snrx10=i;
        ue_info_list[i]->ul_rssi=i;
    }
    // assgin ue info pointer
    ue_list_m->ue_info = ue_info_list;

    return ue_list_m;
}


void ran_read_new(RANParameter ran_par_enum, RANParamMapEntry* map_entry){
    switch (ran_par_enum)
    {
        case RAN_PARAMETER__GNB_ID:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_STRING_VALUE;
            map_entry->string_value = mymy_itoa(gnb_id);
            break;
        case RAN_PARAMETER__SOMETHING:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_STRING_VALUE;
            map_entry->string_value = mymy_itoa(something);
            break;
        case RAN_PARAMETER__UE_LIST:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_UE_LIST;
            map_entry->ue_list = get_ue_list();
            break;
        default:
            printf("unrecognized param %d\n",ran_par_enum);
            assert(0!=0);
    }
}

uint32_t get_nr_rx_total_gain_dB (module_id_t Mod_id,uint8_t CC_id)
{

    PHY_VARS_NR_UE *ue = PHY_vars_UE_g[Mod_id][CC_id];

    if (ue)
        return ue->rx_total_gain_dB;

    return 0xFFFFFFFF;
}

float_t get_nr_RSRP(module_id_t Mod_id,uint8_t CC_id,uint8_t gNB_index)
{

    AssertFatal(PHY_vars_UE_g!=NULL,"PHY_vars_UE_g is null\n");
    AssertFatal(PHY_vars_UE_g[Mod_id]!=NULL,"PHY_vars_UE_g[%d] is null\n",Mod_id);
    AssertFatal(PHY_vars_UE_g[Mod_id][CC_id]!=NULL,"PHY_vars_UE_g[%d][%d] is null\n",Mod_id,CC_id);

    PHY_VARS_NR_UE *ue = PHY_vars_UE_g[Mod_id][CC_id];

    if (ue)
        return (10*log10(ue->measurements.rsrp[gNB_index])-
                get_nr_rx_total_gain_dB(Mod_id,0) -
                10*log10(20*12));
    return -140.0;
}