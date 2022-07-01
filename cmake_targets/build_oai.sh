#!/bin/bash
#/*
# * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# * contributor license agreements.  See the NOTICE file distributed with
# * this work for additional information regarding copyright ownership.
# * The OpenAirInterface Software Alliance licenses this file to You under
# * the OAI Public License, Version 1.1  (the "License"); you may not use this file
# * except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.openairinterface.org/?page_id=698
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *-------------------------------------------------------------------------------
# * For more information about the OpenAirInterface (OAI) Software Alliance:
# *      contact@openairinterface.org
# */

# file build_oai
# brief OAI automated build tool that can be used to install, compile, run OAI.
# author  Navid Nikaein, Lionel GAUTHIER, Laurent Thomas

set -e

# Include helper functions
ORIGIN_PATH=$PWD
THIS_SCRIPT_PATH=$(dirname $(readlink -f $0))
source $THIS_SCRIPT_PATH/tools/build_helper

# Set environment variables (OPENAIR_HOME, ...)
set_openair_env

# Variables for UE data generation
gen_nvram_path=$OPENAIR_DIR/targets/bin
conf_nvram_path=$OPENAIR_DIR/openair3/NAS/TOOLS/ue_eurecom_test_sfr.conf

MSC_GEN=0
XFORMS="True"
SKIP_SHARED_LIB_FLAG="False"
UE_EXPANSION="False"
PRINT_STATS="False"
VCD_TIMING="False"
DEADLINE_SCHEDULER_FLAG_USER=""
CPU_AFFINITY_FLAG_USER="False" #Only valid when low-latency flag is set to False
REL="Rel15"
HW="None"
TP="Ethernet"
EPC=0
VERBOSE_CI=0
VERBOSE_COMPILE=0
CFLAGS_PROCESSOR_USER=""
RUN_GROUP=0
TEST_CASE_GROUP=""
BUILD_DOXYGEN=0
BUILD_COVERITY_SCAN=0
T_TRACER="True"
DISABLE_HARDWARE_DEPENDENCY="False"
CMAKE_BUILD_TYPE=""
CMAKE_CMD="$CMAKE"
UE_AUTOTEST_TRACE="False"
UE_DEBUG_TRACE="False"
UE_TIMING_TRACE="False"
USRP_REC_PLAY="False"
BUILD_ECLIPSE=0
NR="False"
ITTI_SIM="False"
SANITIZE_ADDRESS="False"
OPTIONAL_LIBRARIES="telnetsrv enbscope uescope nrscope msc"
RU=0
trap handle_ctrl_c INT


function print_help() {
  echo_info "
This program installs OpenAirInterface Software
You should have ubuntu 16.xx or 18.04 updated
Options
-c | --clean
   Erase all files to make a rebuild from start
-C | --clean-all
   Erase all files made by previous compilations, installations
--clean-kernel
   Erase previously installed features in kernel: iptables, drivers, ...
-I | --install-external-packages
   Installs required packages such as LibXML, asn1.1 compiler, freediameter, ...
   This option will require root password
--install-optional-packages
   Install useful but not mandatory packages such as valgrind
-i | --install-system-files
   Install OpenAirInterface required files in Linux system
   This option will require root password
-g | --run-with-gdb <Release | RelWithDebInfo | MinSizeRel | Debug
  specify the build mode used by cmake. defaults to Debug mode if -g is used alone, with no mode parameter
  if -g is not specifies, Release mode is used.
-G | --cmaketrace
   enable cmake debugging messages
--eNB
  Makes the LTE softmodem
--eNBocp
  Makes the OCP LTE softmodem
-gNB
  Makes the NR softmodem
--nrUE
  Makes the NR UE softmodem
--RU
  Makes the OAI RRU (without full stack)
--UE
   Makes the UE specific parts (ue_ip, usim, nvram) from the given configuration file
--UE-conf-nvram [configuration file]
   Specify conf_nvram_path (default \"$conf_nvram_path\")
--UE-gen-nvram [output path]
   Specify gen_nvram_path (default \"$gen_nvram_path\")
--HWLAT
    Makes test program for haw latency tests
-a | --agent
   Enables agent for software-defined control of the eNB
-w | --hardware
   EXMIMO, USRP, BLADERF, LMSSDR, IRIS, ADRV9371_ZC706, SIMU, None (Default)
   Adds this RF board support (in external packages installation and in compilation)
-P | --phy_simulators
   Makes the unitary tests Layer 1 simulators
-S | --core_simulators
   Makes the core security features unitary simulators
-s | --check
   runs a set of auto-tests based on simulators and several compilation tests
--run-group 
   runs only specified test cases specified here. This flag is only valid with -s
-V | --vcd
   Adds a debgging facility to the binary files: GUI with major internal synchronization events
--verbose-compile
   Shows detailed compilation instructions in makefile
--cflags_processor
   Manually Add CFLAGS of processor if they are not detected correctly by script. Only add these flags if you know your processor supports them. Example flags: -msse3 -msse4.1 -msse4.2 -mavx2
--build-doxygen
   Builds doxygen based documentation.
--build-coverity-scan
   Builds Coverity-Scan objects for upload
--disable-deadline
   Disables deadline scheduler of Linux kernel (>=3.14.x).
--enable-deadline
   Enable deadline scheduler of Linux kernel (>=3.14.x). 
--disable-cpu-affinity
   Disables CPU Affinity between UHD/TX/RX Threads (Valid only when deadline scheduler is disabled). By defaulT, CPU Affinity is enabled when not using deadline scheduler. It is enabled only with >2 CPUs. For eNB, CPU_0-> Device library (UHD), CPU_1->TX Threads, CPU_2...CPU_MAX->Rx Threads. For UE, CPU_0->Device Library(UHD), CPU_1..CPU_MAX -> All the UE threads
--enable-cpu-affinity
--disable-T-Tracer
   Disables the T tracer.
--disable-hardware-dependency
   Disable HW dependency during installation
--ue-autotest-trace
   Enable specific traces for UE autotest framework
--ue-trace
   Enable traces for UE debugging
--ue-timing
   Enable traces for timing
--uhd-images-dir
   Download UHD images in the indicated location
--build-eclipse
   Build eclipse project files.
--build-lib <libraries>
   Build optional shared library, <libraries> can be one or several of $OPTIONAL_LIBRARIES or \"all\"
--usrp-recplay
   Build for I/Q record-playback modes
-k | --skip-shared-libraries
   Skip build for shared libraries to reduce compilation time when building frequently for debugging purposes
--ittiSIM
  Makes the itti simulator
-h | --help
   Print this help

Usage (first build):
 NI/ETTUS B201  + COTS UE : ./build_oai -I -i --eNB -w USRP
Usage (regular):
 Eurecom EXMIMO + OAI ENB : ./build_oai --eNB  
 NI/ETTUS B201  + OAI ENB : ./build_oai --eNB -w USRP"
}


function main() {

  until [ -z "$1" ]
  do
    case "$1" in
      --arch-native)
          echo "arch native selected"
          CMAKE_C_FLAGS+=("-march=native")
          CMAKE_CXX_FLAGS+=("-march=native")
          shift;;
       -c | --clean)
            CLEAN=1
            shift;;
       -C | --clean-all)
            CLEAN_ALL=1
            shift;;
       --clean-kernel)
            clean_kernel
            echo_info "Erased iptables config and removed modules from kernel"
            shift;;
       -I | --install-external-packages)
            INSTALL_EXTERNAL=1
            echo_info "Will install external packages"
            shift;;
       --install-optional-packages)
            INSTALL_OPTIONAL=1
            echo_info "Will install optional packages"
            shift;;
       -i | --install-system-files)
            INSTALL_SYSTEM_FILES=1
            echo_info "Will copy OpenAirInterface files in Linux directories"
            shift;;
       -g | --run-with-gdb)
            case "$2" in
                "Release")
                    GDB=0
                    CMAKE_BUILD_TYPE="Release"
                    echo_info "Will Compile without gdb symbols and with compiler optimization"
                    CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=Release"
                    shift
                    ;;
                "RelWithDebInfo")
                    GDB=0
                    CMAKE_BUILD_TYPE="RelWithDebInfo"
                    echo_info "Will Compile with gdb symbols"
                    CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=1"
                    shift
                    ;;
                "MinSizeRel")
                    GDB=0
                    CMAKE_BUILD_TYPE="MinSizeRel"
                    echo_info "Will Compile for minimal exec size"
                    CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=MinSizeRel"
                    shift
                    ;;
                "Debug" | *)
                    GDB=1
                    CMAKE_BUILD_TYPE="Debug"
                    echo_info "Will Compile with gdb symbols and disable compiler optimization"
                    CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=Debug"
                    if [ "$2" == "Debug" ] ; then
                        shift
                    fi
                    ;;
            esac
            shift;;
       -G | --cmaketrace)
            CMAKE_CMD="$CMAKE_CMD --trace-expand"
            shift;;
       --eNB)
            eNB=1
            RU=0
            echo_info "Will compile eNB"
            shift;;
       --eNBocp)
            eNBocp=1
            RU=0
            echo_info "Will compile OCP eNB"
            shift;;
      --gNB)
            gNB=1
            RU=0
            NR="True"
            echo_info "Will compile gNB"
            shift;;
       --RU)
	          RU=1
            echo_info "Will compile RRU"
	    shift;;
       -a | --agent)
            echo_info "FlexRAN support is always compiled into the eNB"
            shift;;
       --UE)
            RU=0
            UE=1
            echo_info "Will compile UE"
            shift;;
       --nrUE)
            RU=0
            nrUE=1
            rfsimNas=1
            NR="True"
            echo_info "Will compile NR UE"
            shift;;
       --mu)
            UE_EXPANSION="True"
            echo_info "Will compile with UE_EXPANSION"
            shift;;
       --UE-conf-nvram)
            conf_nvram_path=$(readlink -f $2)
            shift 2;;
       --UE-gen-nvram)
            gen_nvram_path=$(readlink -f $2)
            shift 2;;
       -w | --hardware)
            # Use OAI_USRP as the key word USRP is used inside UHD driver           
            case "$2" in
                "EXMIMO")
                    HW="EXMIMO"
                    ;;
                "USRP" | "BLADERF" | "LMSSDR" | "IRIS" | "ADRV9371_ZC706" | "SIMU")
                    HW="OAI_"$2
                    ;;
                "None")
                    HW="None"
                    ;;
                *)
                    echo_fatal "Unknown HW type $HW: exit..."
            esac
			echo_info "Setting hardware to: $HW"
            shift 2;;
       -t | --transport)
	    TP=$2
            shift 2;;
       -P | --phy_simulators)
            SIMUS_PHY=1
	          RU=0
            echo_info "Will compile dlsim, ulsim, ..."
            shift;;
       -S | --core_simulators)
            SIMUS_CORE=1
            echo_info "Will compile security unitary tests"
            shift;;
       -s | --check)
            OAI_TEST=1
            echo_info "Will run auto-tests"
            shift;;
       --run-group)
            RUN_GROUP=1
            TEST_CASE_GROUP=$2
            echo_info "executing test cases only in group: $TEST_CASE_GROUP"
            shift 2;;
       -V | --vcd)
            echo_info "Setting gtk-wave output"
            VCD_TIMING=1
            EXE_ARGUMENTS="$EXE_ARGUMENTS -V"
            shift;;
       -x | --xforms)
            XFORMS=1
            EXE_ARGUMENTS="$EXE_ARGUMENTS -d"
            echo_info "Will generate the software oscilloscope features"
            shift;;
       --HWLAT)
            HWLAT=1
            echo_info "Will compile hw latency test program"
            shift;;
       --HWLAT_TEST)
            HWLAT_TEST=1
            echo_info "Will compile hw latency test program"
            shift;;
       --verbose-ci)
	        VERBOSE_CI=1
            echo_info "Will compile with verbose instructions in CI Docker env"
            shift;;
       --verbose-compile)
	        VERBOSE_COMPILE=1
            echo_info "Will compile with verbose instructions"
            shift;;
       --cflags_processor)
            CFLAGS_PROCESSOR_USER=$2
            echo_info "Setting CPU FLAGS from USER to: $CFLAGS_PROCESSOR_USER"
            shift 2;;
       --build-doxygen)
	        BUILD_DOXYGEN=1
            echo_info "Will build doxygen support"
            shift;;     
       --build-coverity-scan)
            BUILD_COVERITY_SCAN=1
            echo_info "Will build Coverity-Scan objects for upload"
            shift;;
       --disable-deadline)
            DEADLINE_SCHEDULER_FLAG_USER="False"
            echo_info "Disabling the usage of deadline scheduler"
            shift 1;;
       --enable-deadline)
            DEADLINE_SCHEDULER_FLAG_USER="True"
            echo_info "Enabling the usage of deadline scheduler"
            shift 1;;
       --enable-cpu-affinity)
            CPU_AFFINITY_FLAG_USER="True"
            echo_info "Enabling CPU Affinity (only valid when not using deadline scheduler)"
            shift 1;;
       --disable-cpu-affinity)
            CPU_AFFINITY_FLAG_USER="False"
            echo_info "Disabling CPU Affinity (only valid when not using deadline scheduler)"
            shift 1;;
       --disable-T-Tracer)
            T_TRACER="False"
            echo_info "Disabling the T tracer"
            shift 1;;
       --disable-hardware-dependency)
            echo_info "Disabling hardware dependency for compiling software"
            DISABLE_HARDWARE_DEPENDENCY="True"
            shift 1;;
       --ue-autotest-trace)
            UE_AUTOTEST_TRACE="True"
            echo_info "Enabling autotest specific trace for UE"
            shift 1;;
       --ue-trace)
            UE_DEBUG_TRACE="True"
            echo_info "Enabling UE trace for debug"
            shift 1;;
       --ue-timing)
            UE_TIMING_TRACE="True"
            echo_info "Enabling UE timing trace"
            shift 1;;
       --uhd-images-dir)
            UHD_IMAGES_DIR=$2
            echo_info "Downloading UHD images in the indicated location"
            shift 2;;
       --build-eclipse)
            BUILD_ECLIPSE=1
            CMAKE_CMD="$CMAKE_CMD"' -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -G"Eclipse CDT4 - Unix Makefiles"'
            echo_info "Enabling build eclipse project support"
            shift 1;;
       --build-lib) 
            BUILD_OPTLIB="" 
            if [  "$2" == "all" ] ; then
              BUILD_OPTLIB="$OPTIONAL_LIBRARIES"
              echo_info "Enabling build of all optional shared libraries ($OPTIONAL_LIBRARIES)" 
            else
              for alib in $2 ; do 
                for oklib in $OPTIONAL_LIBRARIES ; do
                  if [ "$alib" = "$oklib" ] ; then
                    BUILD_OPTLIB="$BUILD_OPTLIB $alib" 
                    echo_info "Enabling build of lib${alib}.so" 
                    fi   
                  done
                done
	          if [ "${BUILD_OPTLIB## }" != "$2" ] ; then
                 echo_fatal "Unknown optional library in $2, valid libraries are $OPTIONAL_LIBRARIES" 
              fi
            fi
            shift 2;;		
        --usrp-recplay)
            USRP_REC_PLAY="True"
            echo_info "Enabling USRP record playback mode"
            shift 1;;
        -k | --skip-shared-libraries)
            SKIP_SHARED_LIB_FLAG="True"
            echo_info "Skipping build of shared libraries, rfsimulator and transport protocol libraries"
            shift;;
    --ninja)
        CMAKE_CMD="$CMAKE_CMD -GNinja"
	MAKE_CMD=ninja
	shift;;
    --sanitize-address | -fsanitize=address)
        SANITIZE_ADDRESS=True
        shift;;
      --ittiSIM)
            ittiSIM=1
            ITTI_SIM="True"
            echo_info "Will compile itti simulator"
            shift;;
        -h | --help)
            print_help
            exit 1;;
	*)
	    print_help
            echo_fatal "Unknown option $1"
            break;;
   esac
  done

  ###################################
  # Check if cov-build is installed #
  ###################################
  if [ "$BUILD_COVERITY_SCAN" == "1" ] ; then
      echo_info "Checking cov-build is installed"
      IS_INSTALLED=`which cov-build | grep -c cov-build || true`
      if [ $IS_INSTALLED -eq 1 ] ; then
          echo_info "Found cov-build"
      else
          echo_fatal "Did NOT find cov-build in PATH!"
      fi
  fi

  CMAKE_CMD="$CMAKE_CMD $CMAKE_C_FLAGS .."
  if [[ ${#CMAKE_C_FLAGS[@]} > 0 ]]; then CMAKE_CMD="$CMAKE_CMD -DCMAKE_C_FLAGS=\"${CMAKE_C_FLAGS[*]}\""; fi
  if [[ ${#CMAKE_CXX_FLAGS[@]} > 0 ]]; then CMAKE_CMD="$CMAKE_CMD -DCMAKE_CXX_FLAGS=\"${CMAKE_CXX_FLAGS[*]}\""; fi

  echo_info "CMAKE_CMD=$CMAKE_CMD"
  echo "CMAKE ready, proceed with build?"
  read -r

  ########################################################
  # Check validity of HW and TP parameters for eNB / gNB #
  ########################################################
  # to be discussed
  
  if [ "$eNB" = "1" -o "$eNBocp" = "1" -o "$gNB" = "1" -o "$RU" = "1" ] ; then
      if [ "$HW" = "None" -a  "$TP" = "None" ] ; then
	      echo_info "No local radio head and no transport protocol selected"
      fi
      if [ "$HW" = "None" ] ; then 
	      echo_info "No radio head has been selected (HW set to $HW)"	
      fi
      if [ "$TP" = "None" ] ; then
	      echo_info "No transport protocol has been selected (TP set to $TP)"	
      fi
  fi
  
  if [ "$HWLAT" = "1" ] ; then
      if [ "$HW" = "None" ] ; then
          echo_info "No radio head has been selected (HW set to $HW)" 
      fi
  fi

  echo_info "RF HW set to $HW" 
  # If the user doesn't specify the Linux scheduler to use, we set a value
  if [ "$DEADLINE_SCHEDULER_FLAG_USER" = "" ]; then
      case "$HW" in
          "EXMIMO")
		      DEADLINE_SCHEDULER_FLAG_USER="True"
		      ;;
          *)
		      DEADLINE_SCHEDULER_FLAG_USER="False"
		      ;;
      esac
  fi

  #Disable CPU Affinity for deadline scheduler
  if [ "$DEADLINE_SCHEDULER_FLAG_USER" = "True" ] ; then 
     CPU_AFFINITY_FLAG_USER="False"
  fi

  echo_info "Flags for Deadline scheduler: $DEADLINE_SCHEDULER_FLAG_USER"
  echo_info "Flags for CPU Affinity: $CPU_AFFINITY_FLAG_USER"

  #######################################################
  # Setting and printing OAI envs, we should check here #
  #######################################################

  echo_info "2. Setting the OAI PATHS ..."

  cecho "OPENAIR_DIR    = $OPENAIR_DIR" $green

  # for conf files copy in this bash script
  if [ -d /usr/lib/freeDiameter ]; then
    export FREEDIAMETER_PREFIX=/usr
  else
    if [ -d /usr/local/lib/freeDiameter ]; then
      export FREEDIAMETER_PREFIX=/usr/local
    else
      echo_warning "FreeDiameter prefix not found, install freeDiameter if EPC, HSS"
    fi
  fi

  if [ "$CLEAN_ALL" = "1" ] ; then
    clean_all_files
    echo_info "Erased all previously producted files"
  fi

  dbin=$OPENAIR_DIR/targets/bin
  dlog=$OPENAIR_DIR/cmake_targets/log
  mkdir -p $dbin $dlog

  if [ "$INSTALL_EXTERNAL" = "1" ] ; then
    echo_info "Installing packages"
    check_install_oai_software
    if [ "$HW" == "OAI_USRP" ] ; then
      echo_info "installing packages for USRP support"
      check_install_usrp_uhd_driver
      if [ ! "$DISABLE_HARDWARE_DEPENDENCY" == "True" ]; then
        install_usrp_uhd_driver $UHD_IMAGES_DIR
      fi
    fi 
#    if [ "$HW" == "OAI_ADRV9371_ZC706" ] ; then
#      echo_info "\nInstalling packages for ADRV9371_ZC706 support"
#      check_install_libiio_driver
#    fi 
    if [ "$HW" == "OAI_BLADERF" ] ; then
      echo_info "installing packages for BLADERF support"
      check_install_bladerf_driver
      if [ ! "$DISABLE_HARDWARE_DEPENDENCY" == "True" ]; then
        flash_firmware_bladerf
      fi
    fi
    if [ "$HW" == "OAI_IRIS" ] ; then
      echo_info "installing packages for IRIS support"
      check_install_soapy
      #if [ ! "$DISABLE_HARDWARE_DEPENDENCY" == "True" ]; then
      #  flash_firmware_iris
      #fi
    fi
    echo_info "Installing protobuf/protobuf-c for flexran agent support"
    install_protobuf_from_source
    install_protobuf_c_from_source
    echo_success "protobuf/protobuf-c installation successful"
  fi

  if [ "$INSTALL_OPTIONAL" = "1" ] ; then
    echo_info "Installing optional packages"
    check_install_additional_tools
  fi

  
  
  echo_info "3. building the compilation directives ..."

  DIR=$OPENAIR_DIR/cmake_targets
  if [ "$SIMUS_PHY" = "1" -o "$SIMUS_CORE" = "1" ] ; then
    build_dir=phy_simulators
  else
    if [ "$T_TRACER" =  "False" ] ; then
      build_dir=ran_build_noLOG
    else
      build_dir=ran_build  
    fi
  fi
  [ "$CLEAN" = "1" ] && rm -rf $DIR/$build_dir/build
  mkdir -p $DIR/$build_dir/build

# configuration module libraries, one currently available, using libconfig 
  config_libconfig_shlib=params_libconfig
  
  # first generate the CMakefile in the right directory
  if [ "$eNB" = "1" -o "$eNBocp" = "1" -o "$UE" = "1" -o "$gNB" = "1" -o "$RU" = "1" -o "$nrUE" = "1" -o "$HW" = "EXMIMO" -o "$ittiSIM" = "1" -o "$rfsimNas" = "1" ] ; then

    # softmodem compilation

    cmake_file=$DIR/$build_dir/CMakeLists.txt
    echo "cmake_minimum_required(VERSION 2.8)"                             >  $cmake_file
    echo "project (OpenAirInterface)"                                    >> $cmake_file
    echo "set ( CMAKE_BUILD_TYPE $CMAKE_BUILD_TYPE )"                     >> $cmake_file
    echo "set ( CFLAGS_PROCESSOR_USER \"$CFLAGS_PROCESSOR_USER\" )"       >> $cmake_file
    echo "set ( UE_EXPANSION $UE_EXPANSION )"                             >> $cmake_file
#    echo "set ( PHY_TX_THREAD $UE_EXPANSION )"                            >> $cmake_file
    echo "set ( PRE_SCD_THREAD $UE_EXPANSION )"                           >> $cmake_file
    echo "set ( ENABLE_VCD_FIFO $VCD_TIMING )"                            >> $cmake_file
    echo "set ( RF_BOARD \"${HW}\")"                                      >> $cmake_file
    echo "set ( TRANSP_PRO \"${TP}\")"                                    >> $cmake_file
    echo "set ( PACKAGE_NAME \"${exec}\")"                                >> $cmake_file
    echo "set ( DEADLINE_SCHEDULER \"${DEADLINE_SCHEDULER_FLAG_USER}\" )" >> $cmake_file
    echo "set ( CPU_AFFINITY \"${CPU_AFFINITY_FLAG_USER}\" )"             >> $cmake_file
    echo "set ( T_TRACER $T_TRACER )"                                     >> $cmake_file
    echo "set ( UE_AUTOTEST_TRACE $UE_AUTOTEST_TRACE )"                   >> $cmake_file
    echo "set ( UE_DEBUG_TRACE $UE_DEBUG_TRACE )"                         >> $cmake_file
    echo "set ( UE_TIMING_TRACE $UE_TIMING_TRACE )"                       >> $cmake_file
    echo "set ( USRP_REC_PLAY $USRP_REC_PLAY )"                           >> $cmake_file
    echo "set ( SKIP_SHARED_LIB_FLAG $SKIP_SHARED_LIB_FLAG )"             >> $cmake_file
    echo "set ( RU $RU )"                                                 >> $cmake_file
    echo "set ( ITTI_SIM $ITTI_SIM )"                                     >> $cmake_file
    echo "set ( SANITIZE_ADDRESS $SANITIZE_ADDRESS )"                     >> $cmake_file
    echo 'include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)'         >> $cmake_file
    cd  $DIR/$build_dir/build
    eval $CMAKE_CMD

    execlist=""
    if [ "$eNB" = "1" ] ; then
      execlist="$execlist lte-softmodem"
    fi
    if [ "$eNBocp" = "1" ] ; then
      execlist="$execlist ocp-enb"
    fi
    if [ "$gNB" = "1" ] ; then
      execlist="$execlist nr-softmodem"
    fi
    if [ "$RU" = "1" ] ; then
      execlist="$execlist oairu"
    fi
    if [ "$UE" = 1 ] ; then
      execlist="$execlist lte-uesoftmodem"
    fi
    if [ "$nrUE" = 1 ] ; then
      execlist="$execlist nr-uesoftmodem"
    fi

    if [ "$ittiSIM" = "1" ] ; then
      execlist="$execlist nr-ittisim"
    fi

    for f in $execlist ; do
      echo_info "Compiling $f..."
      compilations \
        $build_dir $f \
        $f $dbin/$f.$REL
    done


# mandatory shared libraries common to UE and (e/g)NB

    if [ "$SKIP_SHARED_LIB_FLAG" = "False" ]; then
      echo_info "Building shared libraries common to UE and gNB"

      compilations \
        $build_dir $config_libconfig_shlib \
        lib$config_libconfig_shlib.so $dbin/lib$config_libconfig_shlib.so

      if [ "$RU" = "0" ] ; then

        compilations \
          $build_dir coding \
          libcoding.so $dbin/libcoding.so


      fi
    fi
  fi

  if [ "$UE" = 1 ] ; then

    echo_info "Compiling UE specific part"

    if [ $IS_CONTAINER -eq 0 ]
    then
      echo_info "Building ue_ip module"
      compilations \
        $build_dir ue_ip \
        CMakeFiles/ue_ip/ue_ip.ko $dbin/ue_ip.ko
    else
      echo_info "Bypassing ue_ip build"
    fi #IS_CONTAINER

#    mkdir -p $DIR/at_commands/build
#    cd $DIR/at_commands/build
#    eval $CMAKE_CMD
#    compilations \
#      at_commands at_nas_ue \
#      at_nas_ue $dbin/at_nas_ue

    [ "$CLEAN" = "1" ] && rm -rf $DIR/nas_sim_tools/build
    mkdir -p $DIR/nas_sim_tools/build
    cd $DIR/nas_sim_tools/build

    eval $CMAKE_CMD
    compilations \
      nas_sim_tools usim \
      usim $dbin/usim
    compilations \
      nas_sim_tools nvram \
      nvram $dbin/nvram
    compilations \
      nas_sim_tools conf2uedata \
      conf2uedata $dbin/conf2uedata

    # generate USIM data
    if [ -f $dbin/conf2uedata ]; then
      install_nas_tools $conf_nvram_path $gen_nvram_path
      echo_info "Copying UE specific part to $DIR/$build_dir/build"
      cp -Rvf $dbin/.ue_emm.nvram0 $DIR/$build_dir/build
      cp -Rvf $dbin/.ue.nvram0 $DIR/$build_dir/build
      cp -Rvf $dbin/.usim.nvram0 $DIR/$build_dir/build
    else
      echo_warning "not generated UE NAS files: binaries not found"
    fi
  fi

  if [ "$SIMUS_PHY" = "1" -o "$SIMUS_CORE" = "1" ] ; then
    cd  $DIR/$build_dir/build
    eval $CMAKE_CMD
  fi

  ##################
  # PHY simulators #
  ##################
  if [ "$SIMUS_PHY" = "1" ] ; then
    echo_info "Compiling physical unitary tests simulators"
    # TODO: fix: dlsim_tm4 pucchsim prachsim pdcchsim pbchsim mbmssim
    simlist="dlsim ulsim ldpctest polartest smallblocktest nr_pbchsim nr_dlschsim nr_ulschsim nr_dlsim nr_ulsim nr_pucchsim nr_prachsim"
	# simlist="ldpctest"
    for f in $simlist ; do
      compilations \
        phy_simulators $f \
	    $f $dbin/$f.$REL
    done
    compilations \
      phy_simulators coding \
      libcoding.so $dbin/libcoding.so
#    compilations \
#	   lte-simulators $config_libconfig_shlib \
#	   lib$config_libconfig_shlib.so $dbin/lib$config_libconfig_shlib.so
  fi

  ###################
  # Core simulators #
  ###################
  if [ "$SIMUS_CORE" = "1" ] ; then
    echo_info "Compiling security unitary tests simulators"
    simlist="secu_knas_encrypt_eia1 secu_kenb aes128_ctr_encrypt aes128_ctr_decrypt secu_knas_encrypt_eea2 secu_knas secu_knas_encrypt_eea1 kdf aes128_cmac_encrypt secu_knas_encrypt_eia2"
    for f in $simlist ; do
      compilations \
	    phy_simulators test_$f \
	    test_$f $dbin/test_$f.$REL
    done
  fi

  ####################################
  # EXMIMO drivers & firmware loader #
  ####################################
  if [ "$HW" = "EXMIMO" ] ; then
    echo_info "Compiling Express MIMO 2 board drivers"
    compilations \
      $build_dir openair_rf \
      CMakeFiles/openair_rf/openair_rf.ko $dbin/openair_rf.ko
    compilations \
	  $build_dir updatefw \
	  updatefw $dbin/updatefw
    echo_info "Compiling oarf tools. The logfile for compilation is here: $dlog/oarf.txt"
    make -C $OPENAIR_DIR/cmake_targets/$build_dir/build oarf > $dlog/oarf.txt 2>&1
    cp $OPENAIR_DIR/cmake_targets/$build_dir/build/*.oct $dbin
    if [ -s $dbin/oarf_config_exmimo.oct ] ; then
	  echo_success "oarf tools compiled"
    else
	  echo_error "oarf tools compilation failed"
    fi
    cp $OPENAIR_DIR/cmake_targets/tools/init_exmimo2 $dbin
  fi
  
  ######################
  # Optional libraries #
  ######################
  if [ ! -z "$BUILD_OPTLIB" ] ; then 
     for oklib in $BUILD_OPTLIB ; do
         compilations \
             $build_dir $oklib \
             lib${oklib}.so $dbin/lib${oklib}.so
     done
  fi

  #####################
  # HWLAT compilation #
  #####################
  if [ "$HWLAT" = "1" ] ; then

      hwlat_exec=lte-hwlat
      hwlat_build_dir=lte-hwlat
     
      echo_info "Compiling $hwlat_exec ..."
    
      [ "$CLEAN" = "1" ] && rm -rf $DIR/lte-hwlat/build 
      mkdir -p $DIR/$hwlat_build_dir/build
      cmake_file=$DIR/$hwlat_build_dir/CMakeLists.txt
      echo "cmake_minimum_required(VERSION 2.8)"                             > $cmake_file
      echo "set ( CMAKE_BUILD_TYPE $CMAKE_BUILD_TYPE )"                     >> $cmake_file
      echo "set ( RF_BOARD \"${HW}\")"                                      >> $cmake_file
      echo 'set ( PACKAGE_NAME "\"lte-hwlat\"")'                            >> $cmake_file
      echo "set ( DEADLINE_SCHEDULER \"${DEADLINE_SCHEDULER_FLAG_USER}\" )" >> $cmake_file
      echo "set ( CPU_AFFINITY \"${CPU_AFFINITY_FLAG_USER}\" )"             >> $cmake_file
      echo "set ( HWLAT \"${HWLAT}\" )"                                     >> $cmake_file
      echo 'include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)'         >> $cmake_file    
      cd $DIR/$hwlat_build_dir/build
      cmake ..
      compilations \
          lte-hwlat lte-hwlat \
          lte-hwlat $dbin/lte-hwlat
  fi  
 
  ##########################
  # HWLAT_TEST compilation #
  ##########################
  if [ "$HWLAT_TEST" = "1" ] ; then

      hwlat_test_exec=lte-hwlat-test
      hwlat_test_build_dir=lte-hwlat-test
     
      echo_info "Compiling $hwlat_test_exec ..."
    
      [ "$CLEAN" = "1" ] && rm -rf $DIR/lte-hwlat-test/build 
      mkdir -p $DIR/$hwlat_test_build_dir/build
      cmake_file=$DIR/$hwlat_test_build_dir/CMakeLists.txt
      echo "cmake_minimum_required(VERSION 2.8)"                             > $cmake_file
      echo "set ( CMAKE_BUILD_TYPE $CMAKE_BUILD_TYPE )"                     >> $cmake_file
      echo "set ( RF_BOARD \"${HW}\")"                                      >> $cmake_file
      echo 'set ( PACKAGE_NAME "\"lte-hwlat-test\"")'                       >> $cmake_file
      echo "set ( DEADLINE_SCHEDULER \"${DEADLINE_SCHEDULER_FLAG_USER}\" )" >> $cmake_file
      echo "set ( CPU_AFFINITY \"${CPU_AFFINITY_FLAG_USER}\" )"             >> $cmake_file
      echo "set ( HWLAT \"${HWLAT}\" )"                                     >> $cmake_file
      echo 'include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)'         >> $cmake_file    
      cd $DIR/$hwlat_test_build_dir/build
      cmake ..
      compilations \
          lte-hwlat-test lte-hwlat-test \
          lte-hwlat-test $dbin/lte-hwlat-test
  fi
  
  ####################################################
  # Build RF device and transport protocol libraries #
  ####################################################
  if [ "$eNB" = "1" -o "$eNBocp" = "1" -o "$UE" = "1" -o "$gNB" = "1" -o "$RU" = "1"  -o "$nrUE" = "1" -o "$HWLAT" = "1" ] ; then

      # build RF device libraries
      if [ "$HW" != "None" ] ; then
          rm -f liboai_device.so
          rm -f $dbin/liboai_device.so

          # link liboai_device.so with the selected RF device library
          if [ "$HW" == "EXMIMO" ] ; then
              compilations \
                  $build_dir oai_exmimodevif \
                  liboai_exmimodevif.so $dbin/liboai_exmimodevif.so.$REL

              ln -sf liboai_exmimodevif.so liboai_device.so
              ln -sf $dbin/liboai_exmimodevif.so.$REL $dbin/liboai_device.so
              echo_info "liboai_device.so is linked to EXMIMO device library"
          elif [ "$HW" == "OAI_USRP" ] ; then
              compilations \
                  $build_dir oai_usrpdevif \
                  liboai_usrpdevif.so $dbin/liboai_usrpdevif.so.$REL

              ln -sf liboai_usrpdevif.so liboai_device.so
              ln -sf $dbin/liboai_usrpdevif.so.$REL $dbin/liboai_device.so
              echo_info "liboai_device.so is linked to USRP device library"        
          elif [ "$HW" == "OAI_BLADERF" ] ; then
              if [ -f "/usr/include/libbladeRF.h" ] ; then
                  compilations \
                      $build_dir oai_bladerfdevif \
                      liboai_bladerfdevif.so $dbin/liboai_bladerfdevif.so.$REL
              fi

              ln -sf liboai_bladerfdevif.so liboai_device.so
              ln -sf $dbin/liboai_bladerfdevif.so.$REL $dbin/liboai_device.so
              echo_info "liboai_device.so is linked to BLADERF device library"	 
          elif [ "$HW" == "OAI_LMSSDR" ] ; then
#              if [ -f "/usr/include/libbladeRF.h" ] ; then
                  compilations \
                      $build_dir oai_lmssdrdevif \
                      liboai_lmssdrdevif.so $dbin/liboai_lmssdrdevif.so.$REL
#              fi

              ln -sf liboai_lmssdrdevif.so liboai_device.so
              ln -sf $dbin/liboai_lmssdrdevif.so.$REL $dbin/liboai_device.so
              echo_info "liboai_device.so is linked to LMSSDR device library"	 
          elif [ "$HW" == "OAI_IRIS" ] ; then
              compilations \
                  $build_dir oai_irisdevif \
                  liboai_irisdevif.so $dbin/liboai_irisdevif.so.$REL

              ln -s liboai_irisdevif.so liboai_device.so
              ln -s $dbin/liboai_irisdevif.so.$REL $dbin/liboai_device.so
              echo_info "liboai_device.so is linked to IRIS device library"
          elif [ "$HW" == "OAI_ADRV9371_ZC706" ] ; then
              SYRIQ_KVER=$(uname -r)
              SYRIQ_KMAJ=$(echo $SYRIQ_KVER | sed -e 's/^\([0-9][0-9]*\)\.[0-9][0-9]*\.[0-9][0-9]*.*/\1/')
              SYRIQ_KMIN=$(echo $SYRIQ_KVER | sed -e 's/^[0-9][0-9]*\.\([0-9][0-9]*\)\.[0-9][0-9]*.*/\1/')
#              echo $SYRIQ_KMAJ$SYRIQ_KMIN
              if [ "$SYRIQ_KMAJ$SYRIQ_KMIN" == "319" ] || [ "$SYRIQ_KMAJ$SYRIQ_KMIN" == "410" ] || [ "$SYRIQ_KMAJ$SYRIQ_KMIN" == "415" ] ; then
#                  echo "Kernel $SYRIQ_KMAJ.$SYRIQ_KMIN detected"
                  ln -sf /usr/local/lib/syriq/libadrv9371zc706.so liboai_device.so
              else
                  echo_error "== FAILED == Unexpected Kernel $SYRIQ_KMAJ.$SYRIQ_KMIN"
              fi
              echo_info "liboai_device.so is linked to ADRV9371_ZC706 device library for Kernel $SYRIQ_KMAJ.$SYRIQ_KMIN"
          else
              echo_info "liboai_device.so is not linked to any device library"
          fi
      fi

      #build simulators devices
      if [ "$SKIP_SHARED_LIB_FLAG" = "False" ]; then
          echo_info "Compiling rfsimulator"
          compilations \
              $build_dir rfsimulator \
              librfsimulator.so $dbin/librfsimulator.so.$REL

          echo_info "Compiling tcp_bridge_oai"
          compilations \
              $build_dir tcp_bridge_oai \
              libtcp_bridge_oai.so $dbin/libtcp_bridge_oai.so.$REL
      fi

      #build transport protocol libraries (currently only ETHERNET is available)
      if [ "$SKIP_SHARED_LIB_FLAG" = "False" ]; then
          echo_info "Building transport protocol libraries"
          rm -f liboai_transpro.so
          rm -f $dbin/liboai_transpro.so
	  if [ "$TP" == "Ethernet" ]; then
              compilations \
		  $build_dir oai_eth_transpro \
		  liboai_eth_transpro.so $dbin/liboai_eth_transpro.so.$REL
              ln -sf liboai_eth_transpro.so liboai_transpro.so
              ln -sf $dbin/liboai_eth_transpro.so.$REL $dbin/liboai_transpro.so
              echo_info "liboai_transpro.so is linked to ETHERNET transport"
	  fi
	  if [ "$TP" == "benetel4g" ]; then
              compilations \
		  $build_dir benetel_4g \
		  libbenetel_4g.so $dbin/libbenetel_4g.$REL
              ln -sf libbenetel_4g.so liboai_transpro.so
              ln -sf $dbin/libbenetel_4g.so.$REL $dbin/liboai_transpro.so
              echo_info "liboai_transpro.so is linked to BENETEL4G transport"
	  fi
	  if [ "$TP" == "benetel5g" ]; then
              compilations \
		  $build_dir benetel_5g \
		  libbenetel_5g.so $dbin/libbenetel_5g.$REL
              ln -sf libbenetel_5g.so liboai_transpro.so
              ln -sf $dbin/libbenetel_5g.so.$REL $dbin/liboai_transpro.so
              echo_info "liboai_transpro.so is linked to BENETEL4G transport"
	  fi
      fi
  fi

  ###################
  # Doxygen Support #
  ###################
  if [ "$BUILD_DOXYGEN" = "1" ] ; then
    doxygen_log=$OPENAIR_DIR/cmake_targets/log/doxygen.log
    echo_info "Building Doxygen based documentation. The documentation file is located here: $OPENAIR_DIR/targets/DOCS/html/index.html"
    echo_info "Doxygen generation log is located here: $doxygen_log"
    echo_info "Generating Doxygen files....please wait"
    (
    [ "$CLEAN" = "1" ] && rm -rf $OPENAIR_DIR/cmake_targets/doxygen/build
    mkdir -p $OPENAIR_DIR/cmake_targets/doxygen/build
    cd $OPENAIR_DIR/cmake_targets/doxygen/build
    eval $CMAKE_CMD
    make doc
    ) >& $doxygen_log
  fi

  ##############
  # Auto-tests #
  ##############
  if [ "$OAI_TEST" = "1" ] ; then
    echo_info "10. Running OAI pre commit tests (pre-ci) ..."
    echo_error "These scripts ASSUME that user is in /etc/sudoers and can execute commands without PASSWORD prompt"
    echo_error "Add the following lines in /etc/sudoers file to make your __user_name__ sudo without password prompt"
    echo_error " __your_user_name__ ALL = (ALL:ALL) NOPASSWD: ALL"
    echo_error " __your_user_name__ ALL = (ALL) NOPASSWD: ALL "
    echo_info "The log file for the autotest script for debugging is located here: $OPENAIR_DIR/cmake_targets/autotests/log/autotests.log "
    echo_info "The results of autotests results is located here: $OPENAIR_DIR/cmake_targets/autotests/log/results_autotests.xml "
    echo_info "You can hit CTRL-C at any time to terminate the autotests..."
    echo "Current User Name: $USER"
    read -s -p "Enter Password: " mypassword
    echo -e "\n"
    rm -fr $OPENAIR_DIR/cmake_targets/autotests/log
    mkdir -p $OPENAIR_DIR/cmake_targets/autotests/log
    if [ "$RUN_GROUP" -eq "1" ]; then
        $OPENAIR_DIR/cmake_targets/autotests/run_exec_autotests.bash -g "$TEST_CASE_GROUP" -p $mypassword >& $OPENAIR_DIR/cmake_targets/autotests/log/autotests.log &
    else
        $OPENAIR_DIR/cmake_targets/autotests/run_exec_autotests.bash -p $mypassword >& $OPENAIR_DIR/cmake_targets/autotests/log/autotests.log &
    fi
    wait
  else
    echo_info "10. Bypassing the Tests ..."
    echo_success "BUILD SHOULD BE SUCCESSFUL"
  fi

}

main "$@"
