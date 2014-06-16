#!/bin/bash
# Copyright (C) 2012 Intel Corporation. All rights reserved.
# This script installs Intel(R) Software Products.

function COMPARE_VERSION(){
    local A=$1
    local B=$2
    local COMPARE_RESULT=0
    local INDEX=1
    local CA="1"
    local CB="2"

    if [ $(echo $A | grep -v "\.") ] && [ $(echo $B | grep -v "\.") ]; then
        if [ "$A" -gt "$B" ] ; then
            COMPARE_RESULT=1
        elif [ "$B" -gt "$A" ] ; then
            COMPARE_RESULT=255
        fi
        return $COMPARE_RESULT
    fi

    while [ "$CA" != "" ] && [ "$CB" != "" ] ; do
        CA=$(echo $A | cut -d'.' -f${INDEX})
        CB=$(echo $B | cut -d'.' -f${INDEX})
        if [ "$CA" != "" ] && [ "$CB" = "" ] ; then
                COMPARE_RESULT=1
        elif [ "$CA" = "" ] && [ "$CB" != "" ] ; then
            COMPARE_RESULT=255
        elif [ "$CA" != "" ] && [ "$CB" != "" ] ; then
            if [ "$CA" -gt "$CB" ] ; then
                COMPARE_RESULT=1
            elif [ "$CB" -gt "$CA" ] ; then
                COMPARE_RESULT=255
            fi
            if [ $COMPARE_RESULT -ne 0 ] ; then
                break
            fi
        fi
        INDEX=$(($INDEX+1))
    done
    
    return $COMPARE_RESULT
    
}

go() {
    if [ ! -d "$INSTALL_DIR" ]; then
        mkdir -p "$INSTALL_DIR" &>/dev/null
        if [ $? -ne 0 ]; then
            log "go: ERROR: problems to create the $INSTALL_DIR... installation cancelled"
            return 1
        fi
    fi

    # comparing versions
    local cur_ver="1.0.025"
    local inst_ver="$(sed 's/VERSION=//g' $INSTALL_DIR/support.txt 2>/dev/null)"

    if [ ! -z "$inst_ver" ]; then
        log "go: installed version of ISM found = $inst_ver"
        COMPARE_VERSION "$cur_ver" "$inst_ver"
        if [ $? -eq 1 ]; then # more
            log "go: current version is newer than existing one... uninstalling old version"
            rm -rf $INSTALL_DIR/Application.properties $INSTALL_DIR/lib $INSTALL_DIR/*.jar $INSTALL_DIR/ism $INSTALL_DIR/*.txt &>/dev/null
        else
            log "go: current version is older or equal to existing one...skipping installation of current version"
            return 0
        fi
    else
        log "go: no installed versions found in $INSTALL_DIR"
    fi
    
    # unpacking ISM
    log -n "go: unpacking $ZIP_TO_INSTALL to $INSTALL_DIR... "
    tar -xzvf "$ZIP_TO_INSTALL" -C "$INSTALL_DIR" &>/dev/null
    if [ $? -ne 0 ]; then
        log "FAIL"
    else
        log "OK"
        # ISM registration in FNC db
        register_ism 
    fi
    
    log "go: successfully installed"
}

register_ism() {
    log "register_ism: registering ISM..."
    which java &>/dev/null
    if [ $? -eq 0 ]; then
        local ism_reg_db="/tmp/intel.ism.reg.db"
        local reg_info="GUID={CB2AA2C3-F557-4397-A0BD-F4F47A10F842};VERSION=1.0;\
URL=https://fnc-notification.intel.com;SN=NONE;ProductName=Intel(R) Software Manager;\
MediaID=NONE;UpdateID=l_isum_p_1.0.025;ProductID=NONE;FulfillmentID=FID-8888888888;"
        
        log "register_ism: reg_info = $reg_info"
        echo $reg_info > $ism_reg_db
        if [ -f "$ism_reg_db" ]; then
            local java_options="-jar ${INSTALL_DIR}/regtool.jar -isroot $LI_IS_ROOT -db $ism_reg_db"
            if [ -n $HOME ]; then
                java_options="-Duser.home=$HOME $java_options"
            fi
            java $java_options
            if [ $? -eq 0 ]; then
                log "register_ism: OK"
            else
                log "register_ism: FAIL"
            fi
            rm -rf $ism_reg_db &>/dev/null
        else
            log "register_ism: unable to create $ism_reg_db file with registration data, skipping registration..."
        fi
    else
        log "register_ism: no java detected, skipping registration..."
    fi
}

vars_dump () {
    log "vars_dump: SCRIPT_DIR=$SCRIPT_DIR"
    log "vars_dump: LI_IS_ROOT=$LI_IS_ROOT"
}

log () {
    echo $*
}

: #============================================================================
: # here the script starts

umask 022

LI_IS_ROOT="$1"

# if empty (script called not from PSET), check current user ID
if [ -z "$LI_IS_ROOT" ]; then
    if [ "$(id -u)" = "0" ]; then
        LI_IS_ROOT="yes"
    else
        LI_IS_ROOT="no"
    fi
fi

SCRIPT_DIR=$(cd $(dirname ${0}) 2>/dev/null; pwd)

cd $SCRIPT_DIR &>/dev/null

ZIP_TO_INSTALL="${SCRIPT_DIR}/ism-1.0.025.tgz"

if [ "x${LI_IS_ROOT}" = "xyes" ]; then
    INSTALL_DIR="/opt/intel/ism"
else
    INSTALL_DIR="$HOME/intel/ism"
fi

log "-------- ISM install: start... --------"
go
log "-------- ISM install: end... --------"

cd - &>/dev/null
