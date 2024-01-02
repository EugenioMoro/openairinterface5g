#ifndef NR_UESOFTMODEM_H
#define NR_UESOFTMODEM_H
#include <executables/nr-softmodem-common.h>
#include <executables/softmodem-common.h>
#include "PHY/defs_nr_UE.h"
#include "SIMULATION/ETH_TRANSPORT/proto.h"



#define  CONFIG_HLP_IF_FREQ                "IF frequency for RF, if needed\n"
#define  CONFIG_HLP_IF_FREQ_OFF            "UL IF frequency offset for RF, if needed\n"
#define  CONFIG_HLP_DLSCH_PARA             "number of threads for dlsch processing 0 for no parallelization\n"
#define  CONFIG_HLP_OFFSET_DIV             "Divisor for computing OFDM symbol offset in Rx chain (num samples in CP/<the value>). Default value is 8. To set the sample offset to 0, set this value ~ 10e6\n"
#define  CONFIG_HLP_MAX_LDPC_ITERATIONS    "Maximum LDPC decoder iterations\n"
/***************************************************************************************************************************************/
/* command line options definitions, CMDLINE_XXXX_DESC macros are used to initialize paramdef_t arrays which are then used as argument
   when calling config_get or config_getlist functions                                                                                 */

#define CALIBRX_OPT       "calib-ue-rx"
#define CALIBRXMED_OPT    "calib-ue-rx-med"
#define CALIBRXBYP_OPT    "calib-ue-rx-byp"
#define DBGPRACH_OPT      "debug-ue-prach"
#define NOL2CONNECT_OPT   "no-L2-connect"
#define CALIBPRACH_OPT    "calib-prach-tx"
#define DUMPFRAME_OPT     "ue-dump-frame"

/*------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                            command line parameters defining UE running mode                                              */
/*   optname                     helpstr                paramflags                      XXXptr        defXXXval         type       numelt   */
/*------------------------------------------------------------------------------------------------------------------------------------------*/
// clang-format off
#define CMDLINE_NRUEPARAMS_DESC {  \
  {"usrp-args",                CONFIG_HLP_USRP_ARGS,           0,               .strptr=&usrp_args,                       .defstrval="type=b200",          TYPE_STRING,   0}, \
  {"tx_subdev",                CONFIG_HLP_TX_SUBDEV,           0,               .strptr=&tx_subdev,                       .defstrval=NULL,                 TYPE_STRING,   0}, \
  {"rx_subdev",                CONFIG_HLP_RX_SUBDEV,           0,               .strptr=&rx_subdev,                       .defstrval=NULL,                 TYPE_STRING,   0}, \
  {"single-thread-disable",    CONFIG_HLP_NOSNGLT,             PARAMFLAG_BOOL,  .iptr=&single_thread_flag,                .defintval=1,                    TYPE_INT,      0}, \
  {"dlsch-parallel",           CONFIG_HLP_DLSCH_PARA,          0,               .u8ptr=NULL,                              .defintval=0,                    TYPE_UINT8,    0}, \
  {"offset-divisor",           CONFIG_HLP_OFFSET_DIV,          0,               .uptr=&nrUE_params.ofdm_offset_divisor,   .defuintval=8,                   TYPE_UINT32,   0}, \
  {"max-ldpc-iterations",      CONFIG_HLP_MAX_LDPC_ITERATIONS, 0,               .iptr=&nrUE_params.max_ldpc_iterations,  .defuintval=5,                   TYPE_UINT8,    0}, \
  {"nr-dlsch-demod-shift",     CONFIG_HLP_DLSHIFT,             0,               .iptr=(int32_t *)&nr_dlsch_demod_shift,   .defintval=0,                    TYPE_INT,      0}, \
  {"V" ,                       CONFIG_HLP_VCD,                 PARAMFLAG_BOOL,  .iptr=&vcdflag,                           .defintval=0,                    TYPE_INT,      0}, \
  {"uecap_file",               CONFIG_HLP_UECAP_FILE,          0,               .strptr=&uecap_file,                      .defstrval="./uecap_ports1.xml", TYPE_STRING,   0}, \
  {"rrc_config_path",          CONFIG_HLP_RRC_CFG_PATH,        0,               .strptr=&rrc_config_path,                 .defstrval="./",                 TYPE_STRING,   0}, \
  {"ue-idx-standalone",        NULL,                           0,               .u16ptr=&ue_idx_standalone,               .defuintval=0xFFFF,              TYPE_UINT16,   0}, \
  {"ue-rxgain",                    CONFIG_HLP_UERXG,           0,               .dblptr=&(rx_gain[0][0]),            .defdblval=110,    TYPE_DOUBLE,   0}, \
  {"ue-rxgain-off",                CONFIG_HLP_UERXGOFF,        0,               .dblptr=&rx_gain_off,                .defdblval=0,      TYPE_DOUBLE,   0}, \
  {"ue-txgain",                    CONFIG_HLP_UETXG,           0,               .dblptr=&(tx_gain[0][0]),            .defdblval=0,      TYPE_DOUBLE,   0}, \
  {"ue-nb-ant-rx",                 CONFIG_HLP_UENANTR,         0,               .iptr=&(nrUE_params.nb_antennas_rx),        .defuintval=1,     TYPE_UINT8,    0}, \
  {"ue-nb-ant-tx",                 CONFIG_HLP_UENANTT,         0,               .iptr=&(nrUE_params.nb_antennas_tx),        .defuintval=1,     TYPE_UINT8,    0}, \
  {"ue-scan-carrier",              CONFIG_HLP_UESCAN,          PARAMFLAG_BOOL,  .iptr=&(nrUE_params.UE_scan_carrier),        .defintval=0,      TYPE_INT,      0}, \
  {"ue-fo-compensation",           CONFIG_HLP_UEFO,            PARAMFLAG_BOOL,  .iptr=&(nrUE_params.UE_fo_compensation),     .defintval=0,      TYPE_INT,      0}, \
  {"ue-max-power",                 NULL,                       0,               .iptr=&(tx_max_power[0]),            .defintval=90,     TYPE_INT,      0}, \
  {"A" ,                           CONFIG_HLP_TADV,            0,               .iptr=&(nrUE_params.timing_advance),         .defintval=0,      TYPE_INT,      0}, \
  {"E" ,                           CONFIG_HLP_TQFS,            PARAMFLAG_BOOL,  .iptr=&(nrUE_params.threequarter_fs),       .defintval=0,      TYPE_UINT8,    0}, \
  {"r"  ,                          CONFIG_HLP_PRB_SA,          0,               .iptr=&(nrUE_params.N_RB_DL),                .defintval=106,    TYPE_UINT,     0}, \
  {"ssb",                          CONFIG_HLP_SSC,             0,               .iptr=&(nrUE_params.ssb_start_subcarrier), .defintval=516,    TYPE_UINT16,   0}, \
  {"T" ,                           CONFIG_HLP_TDD,             PARAMFLAG_BOOL,  .iptr=&tddflag,                      .defintval=0,      TYPE_INT,      0}, \
  {"if_freq" ,                     CONFIG_HLP_IF_FREQ,         0,               .u64ptr=&(nrUE_params.if_freq),              .defuintval=0,     TYPE_UINT64,   0}, \
  {"if_freq_off" ,                 CONFIG_HLP_IF_FREQ_OFF,     0,               .iptr=&(nrUE_params.if_freq_off),            .defuintval=0,     TYPE_INT,      0}, \
  {"chest-freq",                   CONFIG_HLP_CHESTFREQ,       0,               .iptr=&(nrUE_params.chest_freq),             .defintval=0,      TYPE_INT,      0}, \
  {"chest-time",                   CONFIG_HLP_CHESTTIME,       0,               .iptr=&(nrUE_params.chest_time),             .defintval=0,      TYPE_INT,      0}, \
  {"ue-timing-correction-disable", CONFIG_HLP_DISABLETIMECORR, PARAMFLAG_BOOL,  .iptr=&(nrUE_params.no_timing_correction),   .defintval=0,      TYPE_INT,      0}, \
  {"mt-mode",                      CONFIG_HLP_MTMODE, PARAMFLAG_BOOL,  .iptr=&(nrUE_params.mt_mode),   .defintval=0,      TYPE_INT,      0}, \
}
// clang-format on

typedef struct {
  uint64_t       optmask;   //mask to store boolean config options
  uint32_t       ofdm_offset_divisor; // Divisor for sample offset computation for each OFDM symbol
  int            max_ldpc_iterations; // number of maximum LDPC iterations
  tpool_t        Tpool;             // thread pool 
  int            UE_scan_carrier;
  int            UE_fo_compensation;
  int            timing_advance;
  uint64_t       if_freq;
  int            if_freq_off;
  int            chest_freq;
  int            chest_time;
  int            no_timing_correction;
  int            nb_antennas_rx;
  int            nb_antennas_tx;
  int            threequarter_fs;
  int            N_RB_DL;
  int            ssb_start_subcarrier;
  int            mt_mode;
} nrUE_params_t;
extern uint64_t get_nrUE_optmask(void);
extern uint64_t set_nrUE_optmask(uint64_t bitmask);
extern nrUE_params_t *get_nrUE_params(void);


// In nr-ue.c
extern int setup_nr_ue_buffers(PHY_VARS_NR_UE **phy_vars_ue, openair0_config_t *openair0_cfg);
extern void fill_ue_band_info(void);
extern void init_NR_UE(int, char*, char*);
extern void init_NR_UE_threads(int);
extern void reset_opp_meas(void);
extern void print_opp_meas(void);
extern void start_oai_nrue_threads(void);
void *UE_thread(void *arg);
void init_nr_ue_vars(PHY_VARS_NR_UE *ue, uint8_t UE_id, uint8_t abstraction_flag);
void init_nrUE_standalone_thread(int ue_idx);
#endif
