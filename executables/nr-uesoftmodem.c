/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */


#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <stdbool.h>
#include <signal.h>

#include "T.h"
#include "assertions.h"
#include "PHY/types.h"
#include "PHY/defs_nr_UE.h"
#include "SCHED_NR_UE/defs.h"
#include "common/ran_context.h"
#include "common/config/config_userapi.h"
//#include "common/utils/threadPool/thread-pool.h"
#include "common/utils/load_module_shlib.h"
//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all
#include "common/utils/nr/nr_common.h"

#include "radio/COMMON/common_lib.h"
#include "radio/ETHERNET/USERSPACE/LIB/if_defs.h"

//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all
#include "openair1/PHY/MODULATION/nr_modulation.h"
#include "PHY/phy_vars_nr_ue.h"
#include "PHY/NR_UE_TRANSPORT/nr_transport_proto_ue.h"
#include "PHY/NR_TRANSPORT/nr_dlsch.h"
//#include "../../SIMU/USER/init_lte.h"

#include "RRC/LTE/rrc_vars.h"
#include "PHY_INTERFACE/phy_interface_vars.h"
#include "NR_IF_Module.h"
#include "openair1/SIMULATION/TOOLS/sim.h"

#ifdef SMBV
#include "PHY/TOOLS/smbv.h"
unsigned short config_frames[4] = {2,9,11,13};
#endif
#include "common/utils/LOG/log.h"
#include "common/utils/LOG/vcd_signal_dumper.h"

#include "UTIL/OPT/opt.h"
#include "enb_config.h"
#include "LAYER2/nr_pdcp/nr_pdcp_oai_api.h"

#include "intertask_interface.h"

#include "PHY/INIT/nr_phy_init.h"
#include "system.h"
#include <openair2/RRC/NR_UE/rrc_proto.h>
#include <openair2/LAYER2/NR_MAC_UE/mac_defs.h>
#include <openair2/LAYER2/NR_MAC_UE/mac_proto.h>
#include <openair2/NR_UE_PHY_INTERFACE/NR_IF_Module.h>
#include <openair1/SCHED_NR_UE/fapi_nr_ue_l1.h>

/* Callbacks, globals and object handlers */

//#include "stats.h"
// current status is that every UE has a DL scope for a SINGLE eNB (eNB_id=0)
#include "PHY/TOOLS/phy_scope_interface.h"
#include "PHY/TOOLS/nr_phy_scope.h"
#include <executables/nr-uesoftmodem.h>
#include "executables/softmodem-common.h"
#include "executables/thread-common.h"

#include "nr_nas_msg_sim.h"
#include <openair1/PHY/MODULATION/nr_modulation.h>
#include "openair2/GNB_APP/gnb_paramdef.h"

extern const char *duplex_mode[];
int mt_mode;
THREAD_STRUCT thread_struct;
nrUE_params_t nrUE_params;

// Thread variables
pthread_cond_t nfapi_sync_cond;
pthread_mutex_t nfapi_sync_mutex;
int nfapi_sync_var=-1; //!< protected by mutex \ref nfapi_sync_mutex
uint16_t sf_ahead=6; //??? value ???
pthread_cond_t sync_cond;
pthread_mutex_t sync_mutex;
int sync_var=-1; //!< protected by mutex \ref sync_mutex.
int config_sync_var=-1;

// not used in UE
instance_t CUuniqInstance=0;
instance_t DUuniqInstance=0;

int get_node_type() {return -1;}

RAN_CONTEXT_t RC;
int oai_exit = 0;


extern int16_t  nr_dlsch_demod_shift;
static int      tx_max_power[MAX_NUM_CCs] = {0};

int      single_thread_flag = 1;
int                 tddflag = 0;
int                 vcdflag = 0;

double          rx_gain_off = 0.0;
char             *usrp_args = NULL;
char             *tx_subdev = NULL;
char             *rx_subdev = NULL;
char       *rrc_config_path = NULL;
char            *uecap_file = NULL;
int               dumpframe = 0;

uint64_t        downlink_frequency[MAX_NUM_CCs][4];
int32_t         uplink_frequency_offset[MAX_NUM_CCs][4];
int             rx_input_level_dBm;

#if MAX_NUM_CCs == 1
rx_gain_t                rx_gain_mode[MAX_NUM_CCs][4] = {{max_gain,max_gain,max_gain,max_gain}};
double tx_gain[MAX_NUM_CCs][4] = {{20,0,0,0}};
double rx_gain[MAX_NUM_CCs][4] = {{110,0,0,0}};
#else
rx_gain_t                rx_gain_mode[MAX_NUM_CCs][4] = {{max_gain,max_gain,max_gain,max_gain},{max_gain,max_gain,max_gain,max_gain}};
double tx_gain[MAX_NUM_CCs][4] = {{20,0,0,0},{20,0,0,0}};
double rx_gain[MAX_NUM_CCs][4] = {{110,0,0,0},{20,0,0,0}};
#endif

// UE and OAI config variables

openair0_config_t openair0_cfg[MAX_CARDS];
int16_t           node_synch_ref[MAX_NUM_CCs];
int               otg_enabled;
double            cpuf;


int          chain_offset = 0;
int           card_offset = 0;
uint64_t num_missed_slots = 0; // counter for the number of missed slots
int     transmission_mode = 1;
int            numerology = 0;
int           oaisim_flag = 0;
int            emulate_rf = 0;
uint32_t       N_RB_DL    = 106;

/* see file openair2/LAYER2/MAC/main.c for why abstraction_flag is needed
 * this is very hackish - find a proper solution
 */
uint8_t abstraction_flag=0;

nr_bler_struct nr_bler_data[NR_NUM_MCS];

static void init_bler_table(char*);

/*---------------------BMC: timespec helpers -----------------------------*/

struct timespec min_diff_time = { .tv_sec = 0, .tv_nsec = 0 };
struct timespec max_diff_time = { .tv_sec = 0, .tv_nsec = 0 };

struct timespec clock_difftime(struct timespec start, struct timespec end) {
  struct timespec temp;

  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }

  return temp;
}

void print_difftimes(void) {
  LOG_I(HW,"difftimes min = %lu ns ; max = %lu ns\n", min_diff_time.tv_nsec, max_diff_time.tv_nsec);
}

int create_tasks_nrue(uint32_t ue_nb) {
  LOG_D(NR_RRC, "%s(ue_nb:%d)\n", __FUNCTION__, ue_nb);
  itti_wait_ready(1);

  if (ue_nb > 0) {
    LOG_I(NR_RRC,"create TASK_RRC_NRUE \n");
    if (itti_create_task (TASK_RRC_NRUE, rrc_nrue_task, NULL) < 0) {
      LOG_E(NR_RRC, "Create task for RRC UE failed\n");
      return -1;
    }
    if (get_softmodem_params()->nsa) {
      init_connections_with_lte_ue();
      if (itti_create_task (TASK_RRC_NSA_NRUE, recv_msgs_from_lte_ue, NULL) < 0) {
        LOG_E(NR_RRC, "Create task for RRC NSA nr-UE failed\n");
        return -1;
      }
    }
    if (itti_create_task (TASK_NAS_NRUE, nas_nrue_task, NULL) < 0) {
      LOG_E(NR_RRC, "Create task for NAS UE failed\n");
      return -1;
    }
  }

  itti_wait_ready(0);

  // Thread to update the RRC timers (in msec) at UE
  pthread_t timers_update;
  threadCreate(&timers_update, nr_rrc_timers_update, NULL, "nr_rrc_timer_update", -1, OAI_PRIORITY_RT_LOW);

  return 0;
}

void exit_function(const char *file, const char *function, const int line, const char *s, const int assert)
{
  int CC_id;

  if (s != NULL) {
    printf("%s:%d %s() Exiting OAI softmodem: %s\n",file,line, function, s);
  }

  oai_exit = 1;

  if (PHY_vars_UE_g && PHY_vars_UE_g[0]) {
    for(CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
      if (PHY_vars_UE_g[0][CC_id] && PHY_vars_UE_g[0][CC_id]->rfdevice.trx_end_func)
        PHY_vars_UE_g[0][CC_id]->rfdevice.trx_end_func(&PHY_vars_UE_g[0][CC_id]->rfdevice);
    }
  }

  if (assert) {
    abort();
  } else {
    sleep(1); // allow lte-softmodem threads to exit first
    exit(EXIT_SUCCESS);
  }
}

uint64_t get_nrUE_optmask(void) {
  return nrUE_params.optmask;
}

uint64_t set_nrUE_optmask(uint64_t bitmask) {
  nrUE_params.optmask = nrUE_params.optmask | bitmask;
  return nrUE_params.optmask;
}

nrUE_params_t *get_nrUE_params(void) {
  return &nrUE_params;
}

static void get_options(void) {

  paramdef_t cmdline_params[] =CMDLINE_NRUEPARAMS_DESC ;
  int numparams = sizeof(cmdline_params)/sizeof(paramdef_t);
  config_get(cmdline_params,numparams,NULL);

  if (vcdflag > 0)
    ouput_vcd = 1;
}

// set PHY vars from command line
void set_options(int CC_id, PHY_VARS_NR_UE *UE){
  NR_DL_FRAME_PARMS *fp = &UE->frame_parms;

  // Init power variables
  tx_max_power[CC_id] = tx_max_power[0];
  rx_gain[0][CC_id]   = rx_gain[0][0];
  tx_gain[0][CC_id]   = tx_gain[0][0];

  // Set UE variables
  UE->rx_total_gain_dB     = (int)rx_gain[CC_id][0] + rx_gain_off;
  UE->tx_total_gain_dB     = (int)tx_gain[CC_id][0];
  UE->tx_power_max_dBm     = tx_max_power[CC_id];
  UE->rf_map.card          = card_offset;
  UE->rf_map.chain         = CC_id + chain_offset;
  UE->max_ldpc_iterations  = nrUE_params.max_ldpc_iterations;
  UE->UE_scan_carrier      = nrUE_params.UE_scan_carrier;
  UE->UE_fo_compensation   = nrUE_params.UE_fo_compensation;
  UE->if_freq              = nrUE_params.if_freq;
  UE->if_freq_off          = nrUE_params.if_freq_off;
  UE->chest_freq           = nrUE_params.chest_freq;
  UE->chest_time           = nrUE_params.chest_time;
  UE->no_timing_correction = nrUE_params.no_timing_correction;
  UE->timing_advance       = nrUE_params.timing_advance;
  mt_mode = nrUE_params.mt_mode;
  

  LOG_I(PHY,"Set UE_fo_compensation %d, UE_scan_carrier %d, UE_no_timing_correction %d \n, chest-freq %d, chest-time %d\n",
        UE->UE_fo_compensation, UE->UE_scan_carrier, UE->no_timing_correction, UE->chest_freq, UE->chest_time);

  // Set FP variables

  if (tddflag){
    fp->frame_type = TDD;
    LOG_I(PHY, "Set UE frame_type %d\n", fp->frame_type);
  }

  fp->nb_antennas_rx       = nrUE_params.nb_antennas_rx;
  fp->nb_antennas_tx       = nrUE_params.nb_antennas_tx;
  fp->threequarter_fs      = nrUE_params.threequarter_fs;
  fp->N_RB_DL              = nrUE_params.N_RB_DL;
  fp->ssb_start_subcarrier = nrUE_params.ssb_start_subcarrier;
  fp->ofdm_offset_divisor  = nrUE_params.ofdm_offset_divisor;

  LOG_I(PHY, "Set UE nb_rx_antenna %d, nb_tx_antenna %d, threequarter_fs %d, ssb_start_subcarrier %d\n", fp->nb_antennas_rx, fp->nb_antennas_tx, fp->threequarter_fs, fp->ssb_start_subcarrier);

}

void init_openair0(void) {
  int card;
  int freq_off = 0;
  NR_DL_FRAME_PARMS *frame_parms = &PHY_vars_UE_g[0][0]->frame_parms;

  for (card=0; card<MAX_CARDS; card++) {
    uint64_t dl_carrier, ul_carrier;
    openair0_cfg[card].configFilename    = NULL;
    openair0_cfg[card].threequarter_fs   = frame_parms->threequarter_fs;
    openair0_cfg[card].sample_rate       = frame_parms->samples_per_subframe * 1e3;
    openair0_cfg[card].samples_per_frame = frame_parms->samples_per_frame;

    if (frame_parms->frame_type==TDD)
      openair0_cfg[card].duplex_mode = duplex_mode_TDD;
    else
      openair0_cfg[card].duplex_mode = duplex_mode_FDD;

    openair0_cfg[card].Mod_id = 0;
    openair0_cfg[card].num_rb_dl = frame_parms->N_RB_DL;
    openair0_cfg[card].clock_source = get_softmodem_params()->clock_source;
    openair0_cfg[card].time_source = get_softmodem_params()->timing_source;
    openair0_cfg[card].tune_offset = get_softmodem_params()->tune_offset;
    openair0_cfg[card].tx_num_channels = min(4, frame_parms->nb_antennas_tx);
    openair0_cfg[card].rx_num_channels = min(4, frame_parms->nb_antennas_rx);

    LOG_I(PHY, "HW: Configuring card %d, sample_rate %f, tx/rx num_channels %d/%d, duplex_mode %s\n",
      card,
      openair0_cfg[card].sample_rate,
      openair0_cfg[card].tx_num_channels,
      openair0_cfg[card].rx_num_channels,
      duplex_mode[openair0_cfg[card].duplex_mode]);

    nr_get_carrier_frequencies(PHY_vars_UE_g[0][0], &dl_carrier, &ul_carrier);

    nr_rf_card_config_freq(&openair0_cfg[card], ul_carrier, dl_carrier, freq_off);
    nr_rf_card_config_gain(&openair0_cfg[card], rx_gain_off);

    openair0_cfg[card].configFilename = get_softmodem_params()->rf_config_file;

    if (usrp_args) openair0_cfg[card].sdr_addrs = usrp_args;
    if (tx_subdev) openair0_cfg[card].tx_subdev = tx_subdev;
    if (rx_subdev) openair0_cfg[card].rx_subdev = rx_subdev;

  }
}

static void init_pdcp(int ue_id) {
  uint32_t pdcp_initmask = (!IS_SOFTMODEM_NOS1) ? LINK_ENB_PDCP_TO_GTPV1U_BIT : (LINK_ENB_PDCP_TO_GTPV1U_BIT | PDCP_USE_NETLINK_BIT | LINK_ENB_PDCP_TO_IP_DRIVER_BIT);

  /*if (IS_SOFTMODEM_RFSIM || (nfapi_getmode()==NFAPI_UE_STUB_PNF)) {
    pdcp_initmask = pdcp_initmask | UE_NAS_USE_TUN_BIT;
  }*/

  if (IS_SOFTMODEM_NOKRNMOD) {
    pdcp_initmask = pdcp_initmask | UE_NAS_USE_TUN_BIT;
  }
  if (get_softmodem_params()->nsa && rlc_module_init(0) != 0) {
    LOG_I(RLC, "Problem at RLC initiation \n");
  }
  nr_pdcp_layer_init();
  nr_pdcp_module_init(pdcp_initmask, ue_id);
  pdcp_set_rlc_data_req_func((send_rlc_data_req_func_t) rlc_data_req);
  pdcp_set_pdcp_data_ind_func((pdcp_data_ind_func_t) pdcp_data_ind);
}

// Stupid function addition because UE itti messages queues definition is common with eNB
void *rrc_enb_process_msg(void *notUsed) {
  return NULL;
}

static bool stop_immediately = false;
static void trigger_stop(int sig)
{
  if (!oai_exit)
    itti_wait_tasks_unblock();
}
static void trigger_deregistration(int sig)
{
  if (!stop_immediately) {
    MessageDef *msg = itti_alloc_new_message(TASK_RRC_UE_SIM, 0, NAS_DEREGISTRATION_REQ);
    itti_send_msg_to_task(TASK_NAS_NRUE, 0, msg);
    stop_immediately = true;
    static const char m[] = "Press ^C again to trigger immediate shutdown\n";
    __attribute__((unused)) int unused = write(STDOUT_FILENO, m, sizeof(m) - 1);
    signal(SIGALRM, trigger_stop);
    alarm(5);
  } else {
    itti_wait_tasks_unblock();
  }
}

static void get_channel_model_mode() {
  paramdef_t GNBParams[]  = GNBPARAMS_DESC;
  config_get(GNBParams, sizeof(GNBParams)/sizeof(paramdef_t), NULL);
  int num_xp_antennas = *GNBParams[GNB_PDSCH_ANTENNAPORTS_XP_IDX].iptr;

  if (num_xp_antennas == 2)
    init_bler_table("NR_MIMO2x2_AWGN_RESULTS_DIR");
  else
    init_bler_table("NR_AWGN_RESULTS_DIR");
}

int NB_UE_INST = 1;

int main( int argc, char **argv ) {
  int set_exe_prio = 1;
  if (checkIfFedoraDistribution())
    if (checkIfGenericKernelOnFedora())
      if (checkIfInsideContainer())
        set_exe_prio = 0;
  if (set_exe_prio)
    set_priority(79);

  //uint8_t beta_ACK=0,beta_RI=0,beta_CQI=2;
  PHY_VARS_NR_UE *UE[MAX_NUM_CCs];
  start_background_system();

  if ( load_configmodule(argc,argv,CONFIG_ENABLECMDLINEONLY) == NULL) {
    exit_fun("[SOFTMODEM] Error, configuration module init failed\n");
  }
  //set_softmodem_sighandler();
  CONFIG_SETRTFLAG(CONFIG_NOEXITONHELP);
  memset(openair0_cfg,0,sizeof(openair0_config_t)*MAX_CARDS);
  memset(tx_max_power,0,sizeof(int)*MAX_NUM_CCs);
  // initialize logging
  logInit();
  // get options and fill parameters from configuration file

  get_options (); //Command-line options specific for NRUE

  get_common_options(SOFTMODEM_5GUE_BIT);
  CONFIG_CLEARRTFLAG(CONFIG_NOEXITONHELP);
#if T_TRACER
  T_Config_Init();
#endif
  initTpool(get_softmodem_params()->threadPoolConfig, &(nrUE_params.Tpool), cpumeas(CPUMEAS_GETSTATE));
  //randominit (0);
  set_taus_seed (0);

  cpuf=get_cpu_freq_GHz();
  itti_init(TASK_MAX, tasks_info);

  init_opt() ;
  load_nrLDPClib(NULL);
 
  if (ouput_vcd) {
    vcd_signal_dumper_init("/tmp/openair_dump_nrUE.vcd");
  }

  #ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "UNKNOWN-EXPERIMENTAL"
#endif
  LOG_I(HW, "Version: %s\n", PACKAGE_VERSION);

  init_NR_UE(1,uecap_file,rrc_config_path);

  int mode_offset = get_softmodem_params()->nsa ? NUMBER_OF_UE_MAX : 1;
  uint16_t node_number = get_softmodem_params()->node_number;
  ue_id_g = (node_number == 0) ? 0 : node_number - 2;
  AssertFatal(ue_id_g >= 0, "UE id is expected to be nonnegative.\n");
  if(IS_SOFTMODEM_NOS1 || get_softmodem_params()->sa || get_softmodem_params()->nsa) {
    if(node_number == 0) {
      init_pdcp(0);
    }
    else {
      init_pdcp(mode_offset + ue_id_g);
    }
  }

  PHY_vars_UE_g = malloc(sizeof(PHY_VARS_NR_UE **));
  PHY_vars_UE_g[0] = malloc(sizeof(PHY_VARS_NR_UE *)*MAX_NUM_CCs);
  if (get_softmodem_params()->emulate_l1) {
    RCconfig_nr_ue_macrlc();
    get_channel_model_mode();
  }

  if (get_softmodem_params()->do_ra)
    AssertFatal(get_softmodem_params()->phy_test == 0,"RA and phy_test are mutually exclusive\n");

  if (get_softmodem_params()->sa)
    AssertFatal(get_softmodem_params()->phy_test == 0,"Standalone mode and phy_test are mutually exclusive\n");

  if (!get_softmodem_params()->nsa && get_softmodem_params()->emulate_l1)
    start_oai_nrue_threads();

  if (!get_softmodem_params()->emulate_l1) {
    for (int CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
      PHY_vars_UE_g[0][CC_id] = (PHY_VARS_NR_UE *)malloc(sizeof(PHY_VARS_NR_UE));
      UE[CC_id] = PHY_vars_UE_g[0][CC_id];
      memset(UE[CC_id],0,sizeof(PHY_VARS_NR_UE));

      set_options(CC_id, UE[CC_id]);
      NR_UE_MAC_INST_t *mac = get_mac_inst(0);

      if (get_softmodem_params()->sa) { // set frame config to initial values from command line and assume that the SSB is centered on the grid
        uint16_t nr_band = get_band(downlink_frequency[CC_id][0],uplink_frequency_offset[CC_id][0]);
        mac->nr_band = nr_band;
        nr_init_frame_parms_ue_sa(&UE[CC_id]->frame_parms,
                                  downlink_frequency[CC_id][0],
                                  uplink_frequency_offset[CC_id][0],
                                  get_softmodem_params()->numerology,
                                  nr_band);
      }
      else{
        DevAssert(mac->if_module != NULL && mac->if_module->phy_config_request != NULL);
        mac->if_module->phy_config_request(&mac->phy_config);
        mac->phy_config_request_sent = true;
        fapi_nr_config_request_t *nrUE_config = &UE[CC_id]->nrUE_config;

        nr_init_frame_parms_ue(&UE[CC_id]->frame_parms, nrUE_config,
        *mac->scc->downlinkConfigCommon->frequencyInfoDL->frequencyBandList.list.array[0]);
      }

      init_nr_ue_vars(UE[CC_id], 0, abstraction_flag);
    }

    init_openair0();
    // init UE_PF_PO and mutex lock
    pthread_mutex_init(&ue_pf_po_mutex, NULL);
    memset (&UE_PF_PO[0][0], 0, sizeof(UE_PF_PO_t)*NUMBER_OF_UE_MAX*MAX_NUM_CCs);
    set_latency_target();

    if(IS_SOFTMODEM_DOSCOPE_QT) {
      load_softscope("nrqt",PHY_vars_UE_g[0][0]);
    }

    if(IS_SOFTMODEM_DOSCOPE) {
      load_softscope("nr",PHY_vars_UE_g[0][0]);
    }

    init_NR_UE_threads(1);
    printf("UE threads created by %ld\n", gettid());
  }

  // wait for end of program
  printf("TYPE <CTRL-C> TO TERMINATE\n");

  if (create_tasks_nrue(1) < 0) {
    printf("cannot create ITTI tasks\n");
    exit(-1); // need a softer mode
  }

  // Sleep a while before checking all parameters have been used
  // Some are used directly in external threads, asynchronously
  sleep(2);
  config_check_unknown_cmdlineopt(CONFIG_CHECKALLSECTIONS);

  // wait for end of program
  printf("Entering ITTI signals handler\n");
  printf("TYPE <CTRL-C> TO TERMINATE\n");
  itti_wait_tasks_end(trigger_deregistration);
  printf("Returned from ITTI signal handler\n");
  oai_exit=1;
  printf("oai_exit=%d\n",oai_exit);

  if (ouput_vcd)
    vcd_signal_dumper_close();

  if (PHY_vars_UE_g && PHY_vars_UE_g[0]) {
    for (int CC_id = 0; CC_id < MAX_NUM_CCs; CC_id++) {
      PHY_VARS_NR_UE *phy_vars = PHY_vars_UE_g[0][CC_id];
      if (phy_vars && phy_vars->rfdevice.trx_end_func)
        phy_vars->rfdevice.trx_end_func(&phy_vars->rfdevice);
    }
  }

  return 0;
}

// Read in each MCS file and build BLER-SINR-TB table
static void init_bler_table(char *env_string) {
  memset(nr_bler_data, 0, sizeof(nr_bler_data));

  const char *awgn_results_dir = getenv(env_string);
  if (!awgn_results_dir) {
    LOG_W(NR_MAC, "No %s\n", env_string);
    return;
  }

  for (unsigned int i = 0; i < NR_NUM_MCS; i++) {
    char fName[1024];
    snprintf(fName, sizeof(fName), "%s/mcs%d_awgn_5G.csv", awgn_results_dir, i);
    FILE *pFile = fopen(fName, "r");
    if (!pFile) {
      LOG_E(NR_MAC, "%s: open %s: %s\n", __func__, fName, strerror(errno));
      continue;
    }
    size_t bufSize = 1024;
    char * line = NULL;
    char * token;
    char * temp = NULL;
    int nlines = 0;
    while (getline(&line, &bufSize, pFile) > 0) {
      if (!strncmp(line, "SNR", 3)) {
        continue;
      }

      if (nlines > NR_NUM_SINR) {
        LOG_E(NR_MAC, "BLER FILE ERROR - num lines greater than expected - file: %s\n", fName);
        abort();
      }

      token = strtok_r(line, ";", &temp);
      int ncols = 0;
      while (token != NULL) {
        if (ncols > NUM_BLER_COL) {
          LOG_E(NR_MAC, "BLER FILE ERROR - num of cols greater than expected\n");
          abort();
        }

        nr_bler_data[i].bler_table[nlines][ncols] = strtof(token, NULL);
        ncols++;

        token = strtok_r(NULL, ";", &temp);
      }
      nlines++;
    }
    nr_bler_data[i].length = nlines;
    fclose(pFile);
  }
}
