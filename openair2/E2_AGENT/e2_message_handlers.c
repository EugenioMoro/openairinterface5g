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
    LOG_E(E2_AGENT,"Not implemented\n");
    assert(0!=0);
    ran_message__free_unpacked(in_mess,NULL);
}
/*
this function just basically prints out the parameters in the request and passes the in_mess to the response generator
*/
void handle_indication_request(RANMessage* in_mess,int out_socket, sockaddr_in peeraddr){
    LOG_I(E2_AGENT,"Indication request for %lu parameters:\n", in_mess->ran_indication_request->n_target_params);
    for(int par_i=0; par_i<in_mess->ran_indication_request->n_target_params; par_i++){
        LOG_I(E2_AGENT,"\tParameter id %d requested (a.k.a %s)\n",\
        in_mess->ran_indication_request->target_params[par_i],\
        get_enum_name(in_mess->ran_indication_request->target_params[par_i]));
    }
    build_indication_response(in_mess, out_socket, peeraddr);
}

/*
this function builds and sends the indication response based on the map inside the in_mess
in_mess is cleared here
*/
void build_indication_response(RANMessage* in_mess, int out_socket, sockaddr_in servaddr){

    RANIndicationResponse rsp = RAN_INDICATION_RESPONSE__INIT;
    RANParamMapEntry **map;
    void* buf;
    unsigned buflen, i;

    // allocate space for the pointers inside the map, which is NULL terminated so it needs 1 additional last pointer
    map = malloc(sizeof(RANParamMapEntry*) * (in_mess->ran_indication_request->n_target_params + 1));

    // now build every element inside the map
    for(i=0; i<in_mess->ran_indication_request->n_target_params; i++){

        // allocate space for this entry and initialize
        map[i] = malloc(sizeof(RANParamMapEntry));
        ran_param_map_entry__init(map[i]);

        // assign key
        map[i]->key=in_mess->ran_indication_request->target_params[i];

        // read the parameter and save it in the map
        ran_read(map[i]->key, map[i]);
    }
    // the map is ready, add the null terminator
    map[in_mess->ran_indication_request->n_target_params] = NULL;

    rsp.n_param_map=in_mess->ran_indication_request->n_target_params;
    rsp.param_map=map;
    buflen = ran_indication_response__get_packed_size(&rsp);
    buf = malloc(buflen);
    ran_indication_response__pack(&rsp,buf);
    LOG_I(E2_AGENT,"Sending indication response\n");
    unsigned slen = sizeof(servaddr);
    int rev = sendto(out_socket, (const char *)buf, buflen,
                     MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                     slen);
    LOG_I(E2_AGENT,"Sent %d bytes, buflen was %u\n",rev, buflen);

    // free map and buffer (rsp not freed because in the stack)
    free_ran_param_map(map);
    free(buf);
    // free incoming ran message
    ran_message__free_unpacked(in_mess,NULL);
}

/*
this function frees a map through introspection, maps !!MUST!! be NULL terminated
*/
void free_ran_param_map(RANParamMapEntry **map){
    int i = 0;
    while(map[i] != NULL){
        // we first need to clear whatever is inside the map entry, we need to consider all the possible value types
        switch(map[i]->value_case){
            case RAN_PARAM_MAP_ENTRY__VALUE_INT64_VALUE:
                // there is no pointer inside the entry to free in this case
                break;
            case RAN_PARAM_MAP_ENTRY__VALUE_STRING_VALUE:
                // free the string and then the entry
                free(map[i]->string_value);
                break;
            case RAN_PARAM_MAP_ENTRY__VALUE_UE_LIST:
                // in this case we free the ue list first
                free_ue_list(map[i]->ue_list);
                break;
            case RAN_PARAM_MAP_ENTRY__VALUE__NOT_SET:
                // nothing to do here, skip to default
            default:
                break;
        }
        // now we can free the entry
        free(map[i]);
        i++;
    }
}

void handle_control(RANMessage* in_mess){
    // loop tarhet params and apply
    for(int i=0; i<in_mess->ran_control_request->n_target_param_map; i++){
        LOG_I(E2_AGENT,"Applying target parameter %s with value %s\n",\
        get_enum_name(in_mess->ran_control_request->target_param_map[i]->key),\
        in_mess->ran_control_request->target_param_map[i]->string_value);
        ran_write(in_mess->ran_control_request->target_param_map[i]);
    }
    // free incoming ran message
    ran_message__free_unpacked(in_mess,NULL);
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
        case RAN_PARAMETER__UE_LIST: // if we receive a ue list message we need to apply its content
            apply_ue_info(target_param_map_entry->ue_list);
            break;
        default:
            LOG_E(E2_AGENT,"ERROR: cannot write RAN, unrecognized target param %d\n", target_param_map_entry->key);
    }
}

void apply_ue_info(UeListM* ue_list){
    LOG_I(E2_AGENT,"in apply_ue_info, ue list size %d\n", ue_list->n_ue_info);
    // loop the ues and apply what needed to each
    for(int ue=0; ue<ue_list->n_ue_info; ue++){
        LOG_I(E2_AGENT,"in apply_ue_info loop ue %d\n",ue);
        // apply gbr
        set_gbr_ue(ue_list->ue_info[ue]->rnti,
            ue_list->ue_info[ue]->tbs_dl_toapply,
            ue_list->ue_info[ue]->tbs_ul_toapply,
            ue_list->ue_info[ue]->is_gbr);

        // more stuff later when needed     
    }
}

void set_gbr_ue(rnti_t rnti, float tbs_dl, float tbs_ul, bool is_GBR){
    LOG_I(E2_AGENT,"in set_gbr_ue\n");
    // acquire mac layer mutex 
    NR_UEs_t *UE_info_gnb = &RC.nrmac[0]->UE_info;
    pthread_mutex_lock(&UE_info_gnb->mutex);

    // iterate ue list until rnti is found
    NR_UE_info_t **UE_list = UE_info_gnb->list;
    bool rnti_not_found = true;
    UE_iterator(UE_list, UE) {
        LOG_I(E2_AGENT,"in set_gbr_ue ue iterator\n");
        if(UE->rnti == rnti){
            LOG_I(E2_AGENT,"in set_gbr_ue rnti found\n");
            // set gbr
            UE->is_GBR = is_GBR;

            // if this ue is gbr, then set tbs size too
            UE->guaranteed_tbs_bytes_dl = tbs_dl;
            UE->guaranteed_tbs_bytes_ul = tbs_ul;
            rnti_not_found = false;
            break;
            
        } else {
            continue;
        }
    }
    if(rnti_not_found){
        LOG_E(E2_AGENT, "RNTI %u not found\n", rnti);
    }

    // release mutex
    pthread_mutex_unlock(&UE_info_gnb->mutex);
}

char* int_to_charray(int i){
    int length = (snprintf(NULL, 0,"%d",i)+1);
    char* ret = malloc(length*sizeof(char));
    sprintf(ret, "%d", i);
    return ret;
}

void handle_master_message(void* buf, int buflen, int out_socket, struct sockaddr_in servaddr){
    RANMessage* in_mess = ran_message__unpack(NULL, (size_t)buflen, buf);
    if (!in_mess){
        LOG_E(E2_AGENT,"error decoding received message, printing for debug:\n");
        for(int i=0;i<buflen; i++){
            uint8_t* tempbuf = (uint8_t*) buf;
            LOG_E(E2_AGENT," %hhx ", tempbuf[i]);
        }
        LOG_E(E2_AGENT,"\n");
        return;
    }
    LOG_I(E2_AGENT,"ran message id %d\n", in_mess->msg_type);
    switch(in_mess->msg_type){
        case RAN_MESSAGE_TYPE__SUBSCRIPTION:
            LOG_I(E2_AGENT,"Subcription message received\n");
            handle_subscription(in_mess);
            break;
        case RAN_MESSAGE_TYPE__INDICATION_REQUEST:
            LOG_I(E2_AGENT,"Indication request message received\n");
            handle_indication_request(in_mess, out_socket, servaddr);
            break;
        case RAN_MESSAGE_TYPE__INDICATION_RESPONSE:
            LOG_I(E2_AGENT,"Indication response message received\n");
            build_indication_response(in_mess, out_socket, servaddr);
            break;
        case RAN_MESSAGE_TYPE__CONTROL:
            LOG_I(E2_AGENT,"Control message received\n");
            handle_control(in_mess);
            break;
        default:
            LOG_I(E2_AGENT,"Unrecognized message type\n");
            ran_message__free_unpacked(in_mess,NULL);
            break;
    }
}


UeListM* get_ue_list(){
    // init ue list
    UeListM* ue_list_m = malloc(sizeof(UeListM));
    ue_list_m__init(ue_list_m);

    NR_UEs_t *UE_info_gnb = &RC.nrmac[0]->UE_info;

    // count how many ues are connected
    int num_ues;
    for(num_ues = 0; num_ues < MAX_MOBILES_PER_GNB; num_ues++){
        if(UE_info_gnb->list[num_ues] == NULL){
            break;
        }
    }

    // insert n ues
    ue_list_m->connected_ues = num_ues;
    ue_list_m->n_ue_info = num_ues;
    if(num_ues == 0){
        return ue_list_m;
    }
    NR_UE_info_t* curr_ue;
    // build list of ue_info_m
    UeInfoM** ue_info_list;
    ue_info_list = malloc(sizeof(UeInfoM*)*(num_ues+1)); // allocating space for 1 additional element which will ne NULL (terminator element)
    for(int i = 0; i<num_ues; i++){
        // init list
        ue_info_list[i] = malloc(sizeof(UeInfoM));
        ue_info_m__init(ue_info_list[i]);
        curr_ue = UE_info_gnb->list[i];

        // add rnti
        ue_info_list[i]->rnti = curr_ue->rnti;
        
        // add grb info
        ue_info_list[i]->has_is_gbr = 1;
        ue_info_list[i]->is_gbr = curr_ue->is_GBR;

        // add tbs info
        ue_info_list[i]->has_tbs_avg_dl = 1;
        ue_info_list[i]->tbs_avg_dl = curr_ue->avg_tbs_1s_dl;
        ue_info_list[i]->has_tbs_avg_ul = 1;
        ue_info_list[i]->tbs_avg_ul = curr_ue->avg_tbs_1s_ul;
    }
    // add a null terminator to the list
    ue_info_list[num_ues] = NULL;
    // assgin ue info pointer
    ue_list_m->ue_info = ue_info_list;
    return ue_list_m;
}

// careful, this function leaves dangling pointers - not a big deal in this case though 
void free_ue_list(UeListM* ue_list_m){
    if(ue_list_m->connected_ues > 0){
        // free the ue list content first
        int i=0;
        while(ue_list_m->ue_info[i] != NULL){ // when we reach NULL we have found the terminator (no need to free the terminator because it hasn't been allocated)
            free(ue_list_m->ue_info[i]);
            i++;
        }
        // then free the list
        free(ue_list_m->ue_info);
    }
    // finally free the outer data structure
    free(ue_list_m);
}

void ran_read(RANParameter ran_par_enum, RANParamMapEntry* map_entry){
    switch (ran_par_enum)
    {
        case RAN_PARAMETER__GNB_ID:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_STRING_VALUE;
            map_entry->string_value = int_to_charray(gnb_id);
            break;
        case RAN_PARAMETER__SOMETHING:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_STRING_VALUE;
            map_entry->string_value = int_to_charray(something);
            break;
        case RAN_PARAMETER__UE_LIST:
            map_entry->value_case=RAN_PARAM_MAP_ENTRY__VALUE_UE_LIST;
            map_entry->ue_list = get_ue_list();
            break;
        default:
            LOG_I(E2_AGENT,"unrecognized param %d\n",ran_par_enum);
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