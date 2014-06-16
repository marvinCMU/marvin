#!/bin/bash
# Copyright 2005 - 2012, Intel Corporation, All Rights Reserved.
# This script installs Intel(R) Composer XE 2013 for Linux* and related products.

declare INTEL_SDP_PRODUCTS_DB=intel_sdp_products.db # TBD...
declare INTEL_SDP_PRODUCTS_DB_LOCAL=''
declare NONRPM_DB_MODE=''
declare NONRPM_DB_PREFIX="$HOME/intel"
ERR_OK=0
ERR_RPM_NOT_RELOCATABLE=10
ERR_RPM_LOCK=11
ERR_RPM_INSTALL=12
ERR_RPM_NO_PACKAGE=13
ERR_UNKNOWN_ARC=14
ERR_RPM_UNINSTALL=15
declare RS
declare -a RA
declare CONFIG_FILENAME=''
declare CONFIG=''
declare -a TRAP_TRACK
TRAP_START=0
TRAP_STOP=0
CMD_STR=''
declare -r TELL_WIDTH=80
declare _SET_FMT_=
RPM_CONFIGURED=-1
RPM_CHECK=0
declare _USER_ID_=
PLATFORM=''
IA=''
declare LOG_F=

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
		        COMPARE_RESULT=-1
	        fi # if [ "$A" -gt "$B" ] ; then
		return $COMPARE_RESULT
        fi
       


        while [ "$CA" != "" ] && [ "$CB" != "" ] ; do
		CA=$(echo $A | cut -d'.' -f${INDEX})
		CB=$(echo $B | cut -d'.' -f${INDEX})
		if [ "$CA" != "" ] && [ "$CB" = "" ] ; then
	    		COMPARE_RESULT=1
		elif [ "$CA" = "" ] && [ "$CB" != "" ] ; then
			COMPARE_RESULT=-1
		elif [ "$CA" != "" ] && [ "$CB" != "" ] ; then
	    		if [ "$CA" -gt "$CB" ] ; then
				COMPARE_RESULT=1
			elif [ "$CB" -gt "$CA" ] ; then
				COMPARE_RESULT=-1
	    		fi # if [ "$CA" -gt "$CB" ] ; then
	    		if [ $COMPARE_RESULT -ne 0 ] ; then
				break
	        	fi # if [ "$COMPARE_RESULT" -ne "0" ] ; then
		fi # if [ "$CA" != "" ] && [ "$CB" = "" ] ; then
		INDEX=$(($INDEX+1))
	done #while [ "$CA" != "" ] && [ "$CB" != "" ] ; do
        return $COMPARE_RESULT
	
} # COMPARE_VERSION(){

function PKG_CHECK(){
	local package=$1
	local pack_ver=$2
	local pack_ver_rel=$3
	local pack='';
	local err=$ERR_OK;
	
	local equal=`echo $pack_ver_rel | sed s/[^e]*e[^e]*//g`
	local greater=`echo $pack_ver_rel | sed s/[^g]*g[^g]*//g`
	local less=`echo $pack_ver_rel | sed s/[^l]*l[^l]*//g`

	local seq=1
	local sgt=1
	local slt=1

	if [ "$pack_ver_rel" = ne ] ; then
		pack=`RPM_INFO $package name`
		err=$?
			
		if [ $err -eq ${ERR_RPM_NO_PACKAGE} ]  ; then 
			return ${ERR_OK}; 
		elif [ $err -ne ${ERR_OK} ] ; then
			return $err;
		fi # if [ $err -eq ${ERR_RPM_NO_PACKAGE} ]  ; then
			
		COMPARE_VERSION `RPM_INFO $package version` $pack_ver
			
		if [ $? -ne 0 ] ; then 
			return ${ERR_OK}; 
		fi # if [ $? -ne 0 ] ; then
		
	fi # if [ "$pack_ver_rel" = ne ] ; then

	if [ "$pack_ver_rel" = eq ] || [ "$pack_ver_rel" = ge ] ||[ "$pack_ver_rel" = lt ] ||[ "$pack_ver_rel" = le ] ||[ "$pack_ver_rel" = gt ] ; then 
		pack=`RPM_INFO $package name`; 
		[ $? -eq ${ERR_OK} ] || err=1; 
		if [ $err -eq ${ERR_OK} ] ; then 
			COMPARE_VERSION `RPM_INFO $package version` $pack_ver
			local error=$?

			if test $equal"x" = "x" ; then
				[ $error -eq 0 ] && seq=0 
			fi # if test $equal"x" = "x" ; then
			if test $greater"x" = "x" ; then
				[ $error -eq 1 ] && sgt=0
			fi # if test $greater"x" = "x" ; then
			if [ $less"x" = "x" ] ; then
				[ $error -eq 255 ] && slt=0
			fi # if [ $less"x" = "x" ] ; then
		fi # if [ $err -eq ${ERR_OK} ] ; then
	fi # if [ "$pack_ver_rel" = eq ] || [ "$pack_ver_rel" = ge...
	

	if [ $err -eq ${ERR_OK} ]; then 
		err=1
		case $pack_ver_rel in
			eq ) [ $seq -ne 0 ] || err=0 ;;
			ge ) [ $seq -ne 0 ] || err=0 ; [ $sgt -eq 1 ] || err=0 ;;
			le ) [ $seq -ne 0 ] || err=0 ; [ $slt -eq 1 ] || err=0 ;;
			gt ) [ $seq -eq 1 ] && [ $sgt -eq 0 ] && err=0 ;;
			lt ) [ $seq -eq 1 ] && [ $slt -eq 0 ] && err=0 ;;
		esac # if [ $err -eq ${ERR_OK} ]; then
	fi # if [ $err -eq ${ERR_OK} ]; then

	return $err

} # PKG_CHECK(){

function ARC_GET(){
	arch_tool="uname -m"
	if $($arch_tool | egrep 'i.86' > /dev/null) ; then
		IA='IA32';
		PLATFORM=x32;
	elif $($arch_tool | grep ia64 > /dev/null) ; then 
		IA='IPF'
		PLATFORM=x64
	elif $($arch_tool | grep x86_64 > /dev/null) ; then
		IA='EM64T'
		PLATFORM=x32_64
	else
		LOG "Unknown arch found: $(uname -m)"
		return ${ERR_UNKNOWN_ARC}
	fi # if [ arch | egrep 'i?86' > /dev/null ] ; then
	return ${ERR_OK}
} # ARC_GET(){

function RPM_CONFIG(){
	
	[ $RPM_CHECK -eq 1 ] || RPM_INIT ; local err=$?	# RPM tool hasn't been checked yet, perform the check
	[ $err -eq ${ERR_OK} ] || return $err

	[ ${RPM_CONFIGURED} -eq -1 ] || return ${RPM_CONFIGURED}

	ARC_GET

	local rpms="4.2.1(x64) 4.1 4.0.2 3.0.5 4.2.2(x64)"
	local rpmi='';

	for rpmi in $rpms ; do
		LOG "Check if RPM supports relocateable packages - $rpmi"
		local ver=`echo $rpmi | sed s/\(.*\)//g`
		local arc=`echo $rpmi | sed s/.*\(//g | sed s/\)//g`
		if [ "$arc" = "$PLATFORM" ] || [ $arc"x" = $ver"x" ] ; then
			PKG_CHECK rpm $ver eq
			if [ $? -eq 0 ] ; then
				LOG "Non-relocatable version of RPM. RPM version: $ver, ARC: $arc"
				PREFIX='';
				RPM_CONFIGURED=${ERR_RPM_NOT_RELOCATABLE}
				return ${ERR_RPM_NOT_RELOCATABLE}
			fi # if [ $? -eq 0 ] ; then
		fi # if [ "$arc" = "$PLATFORM" ] ; then
	done # for rpmi in $rpms ; do
	RPM_CONFIGURED=${ERR_OK};
	return ${ERR_OK}
} # RPM_CONFIG(){

function SPLIT() {

    local delimiter="$1"
    local string="$2"
    local ifs="$IFS"
    local -a result

    [[ $# -eq 2 ]] || CROAK "$FUNCNAME() expects 2 arguments."
    [[ "${#delimiter}" -eq 1 ]] || CROAK "$FUNCNAME(): The 1st argument must be a single char."

    IFS="$delimiter"; result=($string); IFS="$ifs"
    RA=( "${result[@]}" )

} # function split

function READ_DEFAULT_ANSWER(){	
	local param=$1
        local non_verbose=$2
	local retvalue=''
	LOG "silent:read_default_answer(): Trying to find the parameter '$param'"
	CONFIG_GET_VALUE $CONFIG_PREFIX $CONFIG_SECTION $param
	retvalue=$RS
	if [ "x$retvalue" == "x" ]; then
	    [[ "$non_verbose" = "1" ]] || WARN "Parameter \"$param\" was not found"
	    RS=''
	else
	    RS=$retvalue
	fi
} #read_default_answer(){
			                                                                                
function SET_DEFAULT_ANSWER(){	
	local question="$1"
	local value="$2"
	
	LOG "Set default answer for:"
	LOG_ARGS "$@"
	if IS_COMMAND_LINE_OPTION_EXISTS silent; then
	    LOG "Failed, silent mode does not support dynamic default answers."
	else
	    CONFIG_SET_VALUE $CONFIG_PREFIX $CONFIG_SECTION $question "$value"
	fi
} #read_default_answer(){

function SET_CONFIG_FILE(){
	if IS_COMMAND_LINE_OPTION_EXISTS silent; then
    	    CONFIG_FILENAME=`GET_COMMAND_LINE_OPTION_VALUE silent` || return 1
	    ABS_PATH $CONFIG_FILENAME
	    CONFIG_FILENAME=$RS
	    CONFIG_READ_FILE $CONFIG_PREFIX $CONFIG_FILENAME
	    LOG "CONFIG is set to '$CONFIG_FILENAME'"
	else
	    LOG "Silent mode was not enabled"
	fi	
} # set_config_file()

function KEEP_ANSWERS(){
	if IS_COMMAND_LINE_OPTION_EXISTS duplicate; then
    	    local dup_filename=`GET_COMMAND_LINE_OPTION_VALUE duplicate`
	    ABS_PATH $dup_filename
	    dup_filename=$RS
	    CONFIG_WRITE_FILE $CONFIG_PREFIX $dup_filename
	    LOG "Duplicate was performed into '$dup_filename'"
	else
	    LOG "Duplicate mode was not enabled"
	fi
}

function IS_ONE_OF() {

    [[ $# -ge 1 ]] || CROAK "$FUNCNAME() expects at least one argument."

    local item="$1"
    shift 1

    local iter
    for iter in "$@"; do
        if [[ "$item" == "$iter" ]]; then
            return 0
        fi
    done
    return 1

} # is_one_of

function MAKE_TEMP_FILE() {

    [[ $# -eq 0 ]] || CROAK "$FUNCNAME() does not expect any arguments."
    LOG_ARGS "$@"
    local temp
    temp=$( RUN mktemp -q "/tmp/install.XXXXXXXX" ) || DIE "Can not create temporary file."
    LOG "ret: <$temp>."
    RS="$temp"

} # create_temp_file

function MAKE_TEMP_DIR() {

    [[ $# -eq 0 ]] || CROAK "$FUNCNAME() does not expect any arguments."
    LOG_ARGS "$@"
    local temp
    temp=$( RUN mktemp -q -d "/tmp/install.XXXXXXXX" ) || DIE "Can not create temporary dir."
    LOG "ret: <$temp>."
    RS="$temp"

} # create_temp_dir

function FIND_STRING()
{
    local s=$1
    local a=
    while [ 1 == 1 ]; do
		shift
		[ "$1" == ";" ] && break
    done
	shift
    LOG "Checking '$s' against this list: $@"
    for a in $@
    do
        if [ "x$s" == "x$a" ];then
            LOG "Found"
            return ${ERR_OK}
        fi
    done
    LOG "Haven't found '$s'"
    return 1
}

function JOIN() {
    local joiner="$1"
    shift 1

    JOIN3 "" "" "$joiner" "$@"

} # function join

function JOIN3() {
    local prefix="$1"
    local suffix="$2"
    local joiner="$3"
    shift 3

    local result=""
    if [[ $# -gt 0 ]]; then
        local first; local -a rest
        first="$prefix$1$suffix"; shift 1      # The first item processed separately.
        rest=( "$@" )                          # Copy all the items (but the first).
        rest=( "${rest[@]/#/$joiner$prefix}" ) # Prepend items with joiner and prefix.
        rest=( "${rest[@]/%/$suffix}" )        # Appemd items with suffix.
        local ifs="$IFS"
        IFS=""
        result="$first${rest[*]}"              # And join all the items together.
        IFS="$ifs"
    fi

    RS="$result"

} # function join3

function DIE() {
    local opts=""
    local log="log"
    local prefix="ERROR: "
    while [[ "$1" == -[a-zA-Z] ]]; do
        case "$1" in
            -L ) log="";;
            -P ) prefix="";;
            *  ) opts="$opt $1";;
        esac
        shift 1
    done
    [[ "$log" != "" ]] && LOG "warn:" "$@" "(end)"
    local message
    for message in "$@"; do
        echo $opts "$prefix$message" 1>&2
    done
    BYE 1
} # function die

function BYE() {

    local code="$1"

    [[ -z "$code" ]] && code="0"
    LOG "bye ($code)."

    exit "$code"

} # function bye

function _croak_() {

    local level=$(( $1 + 2 ))
    shift 1

    DIE "Internal error in function \"${FUNCNAME[$level]}\", file \"${BASH_SOURCE[$level]}\", line ${BASH_LINENO[$level-1]}:" \
        "$@" "Please report."

} # function _croak_

function croak0() {

    _croak_ 0 "$@"

} # function croak0

function CROAK() {

    _croak_ 1 "$@"

} # function croak

function INIT_LOG() {

    if [ "x$LOG_F" == "x" ]; then
	local param=$1
	if [ "x$param" == "x" ]; then
	    WARN -L "LOG file was not specified. Logging will be disabled."
	else
	    LOG_F=$param
	fi
    fi
    
    [ -e "$LOG_F" ] || RUN :>"$LOG_F" 
    echo "-+ Logging in shell wrapper is started with LOG=$LOG_F +-" >> "$LOG_F"
    if [ $? != 0 ]; then
	WARN -L "Can not write log file \"$LOG_F\". Logging will be disabled."
	unset LOG_F
    fi

} # function init_log

function LOG() {

    if [[ -n "$LOG_F" ]]; then
        local time_stamp=$( date +'%a %b %d %T' )
        local line
        for line in "$@"; do
            { echo "$time_stamp: $line" >> "$LOG_F"; } 2> /dev/null
            if [[ $? -ne 0 ]]; then
                WARN -L "Can not write log file \"$LOG_F\"."
                unset LOG_F  # Unset LOG variable not to log any messages any more.
                break
            fi
        done
    fi

} # function log

function LOG_ARGS() {

    local func="${FUNCNAME[1]}"

    JOIN3 "<" ">" ", " "$@"
    if [[ "$RS" == "" ]]; then
        LOG "$func."
    else
        LOG "$func: $RS."
    fi

} # function log_args

function LOG_VARS() {

    local name str
    local -a vars=()
    for name in "$@"; do
        vars["${#vars[@]}"]="$name=\"${!name}\""
    done
    JOIN ", " "${vars[@]}"
    LOG "vars: $RS."

} # function log_vars

function LOG_FILE() {

    local file="$1"
    local line

    LOG \
        "file <$file>:" \
        "+-------------------------------------------------------------------------------"
    if [[ -f "$file" ]]; then
        while read line; do
            LOG "| $line"
        done < "$file"
    else
        LOG "file not found"
    fi
    LOG "+-------------------------------------------------------------------------------"

} # function log_file

function WARN() {
    local opts=""
    local log="log"
    local prefix="WARNING: "
    while [[ "$1" == -[a-zA-Z] ]]; do
        case "$1" in
            -L ) log="";;
            -P ) prefix="";;
            *  ) opts="$opt $1";;
        esac
        shift 1
    done
    [[ "$log" != "" ]] && LOG_ARGS "$@"
    local message
    for message in "$@"; do
        echo $opts "$prefix$message" 1>&2
    done
} # function warn

function RPM_INIT(){
	[ ${RPM_CHECK} -eq 0 ] || return ${ERR_OK}
	rpm -q rpm &> /dev/null

	
	if [ $? -ne 0 ] ; then
		LOG "Cannot get shared lock on RPM Database"
		return ${ERR_RPM_LOCK}
	fi

	RPM_CHECK=1
	
	return ${ERR_OK}
} # RPM_INIT(){   

function SAVE_COMMAND_LINE(){
	CMD_STR=$@
	declare CMD_STR   
} #SAVE_COMMAND_LINE
			                                                                                
function IS_COMMAND_LINE_OPTION_EXISTS(){
                                            
	local cmd=$1                                    
	if echo $CMD_STR | egrep -i "[[:space:]]*\-\-$cmd" &> /dev/null ; then
		return ${ERR_OK}
	fi
	
	return 1
} # is_command_line_option_exists()

function GET_COMMAND_LINE_OPTION_VALUE(){
	
	local cmd=$1
	local err=0;
	
	IS_COMMAND_LINE_OPTION_EXISTS $cmd
	err=$?
	
	[ $err -eq ${ERR_OK} ] || return 1
		
        RS=$(echo $CMD_STR | sed s/.*--$cmd[[:blank:]]*//g | sed 's/[[:blank:]]*--.*$//g')
        [[ -z "$RS" ]] && return 1
        echo $RS
	
} # get_command_line_option_value() {

function NONRPM_SET_DB_MODE(){
    [ "x$NONRPM_DB_MODE" == "x" ] || return 0
    LOG "NONRPM_SET_DB_MODE"

    INTEL_SDP_PRODUCTS_DB="$NONRPM_DB_PREFIX/$INTEL_SDP_PRODUCTS_DB"
    LOG "INTEL_SDP_PRODUCTS_DB set to $INTEL_SDP_PRODUCTS_DB"

    if IS_COMMAND_LINE_OPTION_EXISTS use-new-db; then
	NONRPM_DB_MODE="both"
    elif IS_COMMAND_LINE_OPTION_EXISTS ignore-old-db; then
	NONRPM_DB_MODE="new"
    else
	NONRPM_DB_MODE="old"
    fi
    LOG "NONRPM_DB_MODE set to $NONRPM_DB_MODE"
    
    if IS_COMMAND_LINE_OPTION_EXISTS nonrpm; then 
	if [ -e "$NONRPM_DB_PREFIX" ]; then
	    if [ ! -d "$NONRPM_DB_PREFIX" ]; then
		LOG "$NONRPM_DB_PREFIX exists and this is not a directory"
		DIE "$NONRPM_DB_PREFIX exists and this is not a directory"
	    fi
	else
	    mkdir -p "$NONRPM_DB_PREFIX" &>/dev/null
	    if [ "$?" != "0" ]; then
		LOG "Unable to create a directory $NONRPM_DB_PREFIX"
		DIE "Unable to create a directory $NONRPM_DB_PREFIX"
	    fi
	fi  
    fi
    
    if [ "$NONRPM_DB_MODE" != "old" ]; then
	LOG "Using INTEL_SDP_PRODUCTS_DB_LOCAL="
	if [ "x$INSTALL_HOST_ID" != "x" ]; then
	    LOG "via INSTALL_HOST_ID var"
	    INTEL_SDP_PRODUCTS_DB_LOCAL="$NONRPM_DB_PREFIX/intel_sdp_products_$INSTALL_HOST_ID.db"
	else
	    LOG "via <hostname>"
	    RS=$(hostname)
	    if [ "$?" != "0" ]; then
		WARN "Unable to define host name, 'only-old-db' mode will be used for Non-rpm functionality."
		INTEL_SDP_PRODUCTS_DB_LOCAL=''
		NONRPM_DB_MODE="old"
	    else
		INTEL_SDP_PRODUCTS_DB_LOCAL="$NONRPM_DB_PREFIX/intel_sdp_products_$RS.db"
	    fi
	fi
	LOG "INTEL_SDP_PRODUCTS_DB_LOCAL set to $INTEL_SDP_PRODUCTS_DB_LOCAL"
	if IS_COMMAND_LINE_OPTION_EXISTS nonrpm; then
	    [ -e "$INTEL_SDP_PRODUCTS_DB_LOCAL" ] || RUN echo -n "" 2>/dev/null 1>$INTEL_SDP_PRODUCTS_DB_LOCAL
	fi
    fi
    
    if IS_COMMAND_LINE_OPTION_EXISTS nonrpm; then
	if [ "$NONRPM_DB_MODE" == "old" ] || [ "$NONRPM_DB_MODE" == "both" ]; then 
	    [ -e "$INTEL_SDP_PRODUCTS_DB" ] || RUN echo -n "" 2>/dev/null 1>$INTEL_SDP_PRODUCTS_DB
	fi
    fi
    
    return 0
}

function NONRPM_GET_BUFFER(){
    
    LOG "NONRPM_GET_BUFFER"
    NONRPM_SET_DB_MODE

    if IS_COMMAND_LINE_OPTION_EXISTS nonrpm; then
	if [ "$NONRPM_DB_MODE" == "old" ] || [ "$NONRPM_DB_MODE" == "both" ]; then 
	    [ -e "$INTEL_SDP_PRODUCTS_DB" ] || RUN echo -n "" 2>/dev/null 1>$INTEL_SDP_PRODUCTS_DB
	fi
	if [ "$NONRPM_DB_MODE" != "old" ]; then
	    [ -e "$INTEL_SDP_PRODUCTS_DB_LOCAL" ] || RUN echo -n "" 2>/dev/null 1>$INTEL_SDP_PRODUCTS_DB_LOCAL
	fi
    fi
    
    local result=''
    local rst=1
    case $NONRPM_DB_MODE in
    new)
	LOG "case new"
	result=$(cat "$INTEL_SDP_PRODUCTS_DB_LOCAL" 2>/dev/null)
	rst=$?
	;;
    both)
	LOG "case both"
	result=$(cat "$INTEL_SDP_PRODUCTS_DB" "$INTEL_SDP_PRODUCTS_DB_LOCAL" 2>/dev/null | sort | uniq)
	rst=$?
	;;
    *)
	LOG "case any"
	result=$(cat "$INTEL_SDP_PRODUCTS_DB" 2>/dev/null)
	rst=$?
	;;
    esac

    if IS_COMMAND_LINE_OPTION_EXISTS nonrpm; then
	[ "$rst" == "0" ] || return 1
    fi
    
    LOG "result:"
    LOG "$result"

    RS=$result

    if [ "$NONRPM_DB_MODE" == "old" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	dbsize=$(ls -ls "$INTEL_SDP_PRODUCTS_DB" 2>/dev/null | cut -d" " -f1)
	errcode=$?
	if [ "x$errcode" == "x0" ] ; then
	    if [ "x$dbsize" == "x0" ] ; then
		rm -f "$INTEL_SDP_PRODUCTS_DB" &>/dev/null
	    fi
	fi
    fi
    if [ "$NONRPM_DB_MODE" == "new" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	dbsize=$(ls -ls "$INTEL_SDP_PRODUCTS_DB_LOCAL" 2>/dev/null | cut -d" " -f1)
	errcode=$?
	if [ "x$errcode" == "x0" ] ; then
	    if [ "x$dbsize" == "x0" ] ; then
		rm -f "$INTEL_SDP_PRODUCTS_DB_LOCAL" &>/dev/null
	    fi
	fi
    fi

    return 0
}

function NONRPM_DB_ENTRY_CHECK_SYNTAX() {
    local entry="$1"
    local regexp='<:[^:]*:intel-[a-z_0-9-]+-[0-9]+(\.[0-9]+)+([abpet]u?)?-[0-9]+\.?[a-z_0-9]+\.rpm:[^:]*:[^:]*:>'
    return $(echo "$entry" | grep -E -x "$regexp" > /dev/null) \
        || DIE "Unexpected error."
} # NONRPM_DB_ENTRY_CHECK_SYNTAX

function NONRPM_DB_ENTRY_GET_FIELD() {
    local entry="$1"
    local field="$2"
    RS=''
	if ! NONRPM_DB_ENTRY_CHECK_SYNTAX "$entry"; then
        return 1
    fi
    if [ "$field" -lt 2 ] || [ "$field" -gt 5 ]; then
        return 2
    fi
    local result=$(echo "$entry" | cut -d':' -f"$field") \
        || DIE "Unexpected error"
    RS=$result
    return 0
} # NONRPM_DB_ENTRY_GET_FIELD

function NONRPM_DB_ENTRY_GET_RPMNAME() {
    NONRPM_DB_ENTRY_GET_FIELD "$1" 2
    return $?
} # NONRPM_DB_ENTRY_GET_RPMNAME

function NONRPM_DB_ENTRY_GET_RPMFILE() {
    NONRPM_DB_ENTRY_GET_FIELD "$1" 3
    return $?
} # NONRPM_DB_ENTRY_GET_RPMFILE

function NONRPM_DB_ENTRY_GET_INSTALLDIR() {
    NONRPM_DB_ENTRY_GET_FIELD "$1" 4
    return $?
} # NONRPM_DB_ENTRY_GET_INSTALLDIR

function NONRPM_DB_ENTRY_GET_LOGFILE() {
    NONRPM_DB_ENTRY_GET_FIELD "$1" 5
    return $?
} # NONRPM_DB_ENTRY_GET_LOGFILE

function NONRPM_DB_ENTRY_ADD() {

    LOG "NONRPM_DB_ENTRY_ADD"
    LOG_ARGS $@
    local entry="$1"

    NONRPM_SET_DB_MODE

    if ! NONRPM_DB_ENTRY_CHECK_SYNTAX "$entry"; then
        return 1
    fi

    local db_content=
    if [ "$NONRPM_DB_MODE" == "old" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	db_content=$(cat "$INTEL_SDP_PRODUCTS_DB" 2>/dev/null)
	echo "$entry" > "$INTEL_SDP_PRODUCTS_DB"
	[ "x$db_content" == "x" ] || echo "$db_content" >> "$INTEL_SDP_PRODUCTS_DB"
	if [ $? -ne 0 ]; then
	    return 2
	fi
    fi

    if [ "$NONRPM_DB_MODE" == "new" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	db_content=$(cat "$INTEL_SDP_PRODUCTS_DB_LOCAL" 2>/dev/null)
	echo "$entry" > "$INTEL_SDP_PRODUCTS_DB_LOCAL"
	[ "x$db_content" == "x" ] || echo "$db_content" >> "$INTEL_SDP_PRODUCTS_DB_LOCAL"
	if [ $? -ne 0 ]; then
	    return 2
	fi
    fi
    
    return 0
    
} # NONRPM_DB_ENTRY_ADD

function NONRPM_DB_ENTRY_REMOVE() {

    LOG "NONRPM_DB_ENTRY_REMOVE"
    LOG_ARGS $@
    local entry="$1"

    NONRPM_SET_DB_MODE

    if ! NONRPM_DB_ENTRY_CHECK_SYNTAX "$entry"; then
        return 1
    fi

    if [ "$NONRPM_DB_MODE" == "old" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	cp -p "$INTEL_SDP_PRODUCTS_DB" "$INTEL_SDP_PRODUCTS_DB~" \
	    || DIE "Unable to create backup copy of \"$INTEL_SDP_PRODUCTS_DB\" file."
	grep -F -v -x "$entry" "$INTEL_SDP_PRODUCTS_DB~" > "$INTEL_SDP_PRODUCTS_DB"
	local rc=$?
	[ $rc -le 1 ] || DIE "Unable to overwrite \"$INTEL_SDP_PRODUCTS_DB\" file."
	chmod --reference="$INTEL_SDP_PRODUCTS_DB~" "$INTEL_SDP_PRODUCTS_DB" \
	    || DIE "Unable to change permissions on \"$INTEL_SDP_PRODUCTS_DB\" file."
	rm -f "$INTEL_SDP_PRODUCTS_DB~" &>/dev/null
    fi
    
    if [ "$NONRPM_DB_MODE" == "new" ] || [ "$NONRPM_DB_MODE" == "both" ]; then
	cp -p "$INTEL_SDP_PRODUCTS_DB_LOCAL" "$INTEL_SDP_PRODUCTS_DB_LOCAL~" \
	    || DIE "Unable to create backup copy of \"$INTEL_SDP_PRODUCTS_DB_LOCAL\" file."
	grep -F -v -x "$entry" "$INTEL_SDP_PRODUCTS_DB_LOCAL~" > "$INTEL_SDP_PRODUCTS_DB_LOCAL"
	local rc=$?
	[ $rc -le 1 ] || DIE "Unable to overwrite \"$INTEL_SDP_PRODUCTS_DB_LOCAL\" file."
	chmod --reference="$INTEL_SDP_PRODUCTS_DB_LOCAL~" "$INTEL_SDP_PRODUCTS_DB_LOCAL" \
	    || DIE "Unable to change permissions on \"$INTEL_SDP_PRODUCTS_DB_LOCAL\" file."
	rm -f "$INTEL_SDP_PRODUCTS_DB_LOCAL~" &>/dev/null
    fi
    
    return 0

} # NONRPM_DB_ENTRY_REMOVE

function NONRPM_DB_IS_PACKAGE_INSTALLED() {

    LOG "NONRPM_DB_FIND_FILE_OWNER"
    LOG_ARGS $@
    local package="$1"
    local rc

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    echo $RS | grep ":$package" &>/dev/null
    rc=$?
    if [ $rc -ge 2 ]; then
        DIE "Unexpected error."
	RS=''
    fi

    RS=$rc
    return $rc
} # NONRPM_DB_IS_PACKAGE_INSTALLED

function NONRPM_DB_FIND_FILE_OWNER() {

    LOG "NONRPM_DB_FIND_FILE_OWNER"
    LOG_ARGS $@
    local file="$1"
    local entry
    local log_file
    local owner

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    local buffer=$RS
    for entry in $buffer; do
	NONRPM_DB_ENTRY_GET_LOGFILE "$entry"
        log_file=$RS
        if [ $? -eq 0 ] && [ -f "$log_file" ]; then
            owner=$(grep -F -x -l "$file" "$log_file")
            if [ $? -ge 2 ]; then
                DIE "Unexpected error."
            fi
            if [ -n "$owner" ]; then
                RS=$entry
                return 0
            fi
        fi
    done
    
    RS=''
    return 1

} # NONRPM_DB_FIND_FILE_OWNER

function NONRPM_DB_ENTRY_FIND_BY_RPMNAME() {

    LOG "NONRPM_DB_ENTRY_FIND_BY_RPMNAME"
    LOG_ARGS $@
    local rpmname=$(basename "$1")
    local entry
    local r_entries=

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    local buffer=$RS
    for entry in $buffer; do
	NONRPM_DB_ENTRY_GET_RPMNAME "$entry"
        if [ "$rpmname" == "$RS" ]; then
	    LOG "Entry $entry found"
            RS="$entry"
	    return 0
        fi
    done
    RS=''
    return 1

} # NONRPM_DB_ENTRY_FIND_BY_RPMNAME

function NONRPM_DB_ENTRY_FIND_BY_RPMFILE() {

    LOG "NONRPM_DB_ENTRY_FIND_BY_RPMFILE"
    LOG_ARGS $@
    local rpmfile=$(basename "$1")
    local entry
    local r_entries=

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    local buffer=$RS
    for entry in $buffer; do
	NONRPM_DB_ENTRY_GET_RPMFILE "$entry"
        if [ "$rpmfile" == "$RS" ]; then
	    LOG "Entry $entry found"
            r_entries="$r_entries $entry"
        fi
    done
    RS=$r_entries
    return 0

} # NONRPM_DB_ENTRY_FIND_BY_RPMFILE

function NONRPM_DB_FIND_BY_INSTALLDIR() {

    LOG "NONRPM_DB_FIND_BY_INSTALLDIR"
    LOG_ARGS $@
    local installdir="$1"
    local entry

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    local buffer=$RS
    for entry in $buffer; do
	NONRPM_DB_ENTRY_GET_INSTALLDIR "$entry"
        if [ "$installdir" == "$RS" ]; then
	    LOG "Entry $entry found"
            RS=$entry
            return 0
        fi
    done
    
    RS=''
    return 1

} # NONRPM_DB_FIND_BY_INSTALLDIR

function NONRPM_DB_CHECK_SHARED_FILES() {
    local install_dir="$1"
    local log_file="$2"

    local entry
    local line
    local shared=""

    NONRPM_GET_BUFFER
    if [ "$?" != "0" ]; then
	DIE "Unable to obtain non-rpm DB content"
    fi

    local buffer=$RS
    for entry in $buffer; do
	NONRPM_DB_ENTRY_GET_INSTALLDIR "$entry"
	if [ "$install_dir" == "$RS" ]; then
	    NONRPM_DB_ENTRY_GET_LOGFILE "$entry"
	    [ "x$RS" == "x$log_file" ] || shared="$RS $shared"
	fi
    done
    
    local rt
    tac "$log_file" | \
    while read line; do
	rt=1
	if [ "x$shared" != "x" ]; then
	    for i in "$shared"; do
		$(cat $i | grep $line &>/dev/null)
		rt=$?
		[ $rt == 0 ] && break
	    done
	fi
	if [ $rt == 0 ]; then
	    LOG "Shared violation $line"
	    continue
	fi 
        if [[ -h "$line" || -f "$line" ]]; then
            rm -f "$line"
        elif [ -d "$line" ]; then
            rmdir --ignore-fail-on-non-empty "$line"
        else
            echo "Warning: installed \"$line\" file not found"
        fi	
    done
}

function RUN() {

    [[ $# -ge 1 ]] || CROAK "$FUNCNAME() expects at least one argument."

    local rc
    LOG_ARGS "$@"
    "$@"
    rc=$?
    LOG "ret: $rc."
    return $rc

} # function run

function RPM_INFO(){
	local rpm_name=$1
	local rpm_tag=$2
	
	[ $RPM_CHECK -eq 1 ] || RPM_INIT ; local err=$?
	[ $err -eq ${ERR_OK} ] || return $err
	
	local opt="-q"
	if [ -f "$rpm_name" ]; then
		LOG "Using file query for package - '$rpm_name'"
		opt="-qp"
	fi #if[ -x $rpm_name ]; then
	
	local rpm_out_count=`rpm $opt $rpm_name 2>&1 | wc -l`
	if [ $rpm_out_count -eq 1 ] ; then 
		local info=`rpm $opt $rpm_name --qf %{$rpm_tag} 2>&1`
		if ! echo $info | grep installed &>/dev/null ; then
			echo $info
			LOG "Package search: '$rpm_name' => '$rpm_tag' => '$info' "
			return $ERR_OK;
		fi #if ! echo $info | grep installed &>/dv/null ; then
	else
		local out=
                local i=1
			local info=`rpm $opt $rpm_name --qf %{$rpm_tag}"\n" 2>&1 | sed $i'!d'`
			out="$out$info"
                echo $info
		LOG "Multiple package search: '$rpm_name' => '$rpm_tag' => '$out' "
		return ${ERR_OK}
	fi

	LOG "Package '$rpm_name' not found"
	
	return ${ERR_RPM_NO_PACKAGE}
} # RPM_INFO(){

function CONFIG_CLEAR() {

    local file="$1"

    eval "unset \${!$prefix__*}"  # Unset all the variables starting with prefix.

} # function config_clear

function CONFIG_READ_FILE() {

    LOG_ARGS "$@"
    local clear="yes"

    while [[ "$1" == -[a-zA-Z] ]]; do
        case "$1" in
            -C ) clear="no";;
            *  ) CROAK "$FUNCNAME(): illegal option: $1";;
        esac
        shift 1
    done

    [[ $# -eq 2 ]] || CROAK "$FUNCNAME() expects 2 arguments"

    local prefix="$1"
    local file="$2"

    [[ -e "$file" ]] || DIE "File \"$file\" does not exist."
    [[ -f "$file" ]] || DIE "\"$file\" is not a file."
    [[ -r "$file" ]] || DIE "File \"$file\" is not readable."

    [[ "$clear" == "yes" ]] && CONFIG_CLEAR "$prefix"

    local i=0
    local section
    local name
    local value
    while read line; do
        i=$(( $i + 1 ))
        if [[ "$line" == \#* ]]; then
            : # Skip comments
        elif [[ "$line" == \[*\] ]]; then
            section="$line"
            section="${section#[}"  # Strip opening bracket.
            section="${section%]}"  # Strip closing bracket.
        elif [[ "$line" == *=* ]]; then
            name="${line%%=*}"      # Strip value.
            value="${line#*=}"      # Strip name.
            eval "${prefix}__${section}__${name}=\"\$value\""
        elif [[ "$line" == "" ]]; then
            : # Ignore empty lines.
        else
            DIE "Error in config file \"$file\" at line $i."
        fi
    done < "$file"

} # function config_read_file

function CONFIG_GET_VALUE() {

    local prefix="$1"
    local section="$2"
    local name="$3"

    eval "RS=\"\${${prefix}__${section}__${name}}\""

} # function config_get_value

function CONFIG_SET_VALUE() {

    local prefix="$1"
    local section="$2"
    local name="$3"
    local value="$4"

    eval "${prefix}__${section}__${name}=\"$value\""

} # function config_set_value

function CONFIG_GET_NAMES() {

    local prefix="$1"
    local section="$2"

    eval "RA=( \${!${prefix}__${section}__*} )"
    RA=( "${RA[@]#${prefix}__${section}__}" )   # Strip prefix and section names.

} # function config_get_names

function CONFIG_GET_SECTIONS() {

    local prefix="$1"

    local -a sections

    local -a vars
    eval "vars=( \${!${prefix}__*} )"
    vars=( "${vars[@]#${prefix}__}" )   # Strip prefix.
    vars=( "${vars[@]%%__*}" )          # Strip names.

    local var
    sections=()
    for var in "${vars[@]}"; do
        if ! IS_ONE_OF "$var" "${sections[@]}"; then
	    sections[${#sections[@]}]="$var"
        fi
    done

    RA=( "${sections[@]}" )

} # function config_get_sections

function CONFIG_WRITE_FILE() {

    local prefix="$1"
    local file="$2"

    [[ -e "$file" ]] || $(echo "" 2>/dev/null 1>$file) || DIE "Cannot create duplicate file \"$file\"."
    [[ -f "$file" ]] || DIE "Duplicate path \"$file\" is not a file."
    [[ -w "$file" ]] || DIE "Duplicate file \"$file\" is not writable."

    {
        local -a sections names
        local section name value
        CONFIG_GET_SECTIONS "$prefix"; sections=( "${RA[@]}" )

        for section in "${sections[@]}"; do
            echo "[$section]"
            CONFIG_GET_NAMES "$prefix" "$section"; names=( "${RA[@]}" )
            for name in "${names[@]}"; do
                CONFIG_GET_VALUE "$prefix" "$section" "$name"; value="$RS"
                echo "$name=$value"
            done
            echo ""
        done
    } > "$file"

} # function config_write_file

function ABS_PATH() {

    local path="$1"
    local cwd

    if [[ "${path:0:1}" != "/" ]]; then
        cwd=$( pwd ) || DIE "Cannot get name of current directory (?!)."
        if [[ "${cwd:-1:1}" == "/" ]]; then
            path="$cwd$path"
        else
            path="$cwd/$path"
        fi
    fi
    RS="$path"

} # function abs_path

function REL_TO_ABS(){
    if echo "$1" | egrep -e ";" -e "'" &>/dev/null; then
        return 1
    fi

    local rv=
    eval local path=$1
    local link_skip_mode="$2"
    local global_flag=
    local resolved_link=

    [ "x$link_skip_mode" == "x" ] && link_skip_mode=0

    local IS_VALID=`echo "$path" | grep "^/"`
    if [ -z "$IS_VALID" ]; then
        path=$(pwd)"/$path"
    fi    

    path=$(echo "$path" | sed 's/\/\/*/\//g')

    local last_dir="$(basename "$path")"
    local prev_dir=$(dirname "$path")
    if [ ! -d "$path" ]; then
        if [ ! -d "$prev_dir" ]; then
            REL_TO_ABS "$prev_dir"
            last_dir=$(echo "$last_dir" | sed 's/\/*$//')
	    RS="$RS/$last_dir"
        else
            last_dir=$(echo "$last_dir" | sed 's/\/*$//')
            GET_PATH_TO_DIRECTORY "$prev_dir"
            if [ "$RS" = "/" ]; then
                RS="$RS$last_dir"
            else
                RS="$RS/$last_dir"
            fi
            rv=0
        fi
    else
        GET_PATH_TO_DIRECTORY "$path"        
        rv=$?
    fi

    if [ "$rv" = "0" ] && [ "$link_skip_mode" == "0" ]; then
        while [ 0 ]; do
	    if [[ -L "$RS" ]]; then
		resolved_link=$(readlink "$RS")
		global_flag=$(echo "$resolved_link" | grep "^/" 2>/dev/null)
		if [ -z "$global_flag" ]; then
		    RS="$(dirname "$RS")/$resolved_link"
		else
		    RS="$resolved_link"
		fi
	    else
		break
	    fi
	done
        prev_dir=$(dirname "$RS")
        last_dir=$(basename "$RS")
        while [ "$prev_dir" != "/" ]; do
    	    if [[ -L "$prev_dir" ]]; then
		resolved_link=$(readlink "$prev_dir")
		global_flag=$(echo "$resolved_link" | grep "^/" 2>/dev/null)
		if [ -z "$global_flag" ]; then
		    prev_dir="$(dirname "$prev_dir")/$resolved_link"
		else
		    prev_dir="$resolved_link"
		fi
	    fi
            last_dir="$(basename "$prev_dir")/$last_dir"
            prev_dir=$(dirname "$prev_dir")
        done
        RS="$prev_dir$last_dir"
	REL_TO_ABS "$RS" 1
    fi
    return $rv
}

function GET_PATH_TO_DIRECTORY() {
    local path="$1"
    local prev_dir=
    local last_dir=
    (cd "$path" &>/dev/null)
     if [ "$?" = "0" ]; then
         path=$(cd "$path" &>/dev/null; pwd)
     else
         prev_dir=$(dirname "$path")
         last_dir=$(basename "$path")
         local flag=0;
         while [ "$flag" != "1" ]; do
             (cd "$prev_dir" &>/dev/null)
             [[ "$?" = "0" ]] && path="$prev_dir/$last_dir" && flag=1
             last_dir="$(basename "$prev_dir")/$last_dir"
             prev_dir=$(dirname "$prev_dir")
         done  
     fi
     RS="$path"
     if [ "$path" != "/" ]; then
         return 0
     else
         return 1
     fi
}

function SIGTRAP_TRACK_ACTION(){
	local action=$1
	local options="$action "

	shift
	while [ ! -z "$1" ] ; do
		options="$options \"$1\""
		shift
	done # while [ ! -z $1 ] ; do
	if [ ${#TRAP_TRACK[*]} -gt 0 ] ; then
		local trip=$((${#TRAP_TRACK[*]}-1))
		[ "${TRAP_TRACK[$trip]}" != "$options" ] || return
	fi
	
	trip=${#TRAP_TRACK[*]}
	
	TRAP_TRACK[$trip]="$options"
	TRAP_STOP=$trip
	
} #SIGTRAP_TRACK_ACTION()

function SIGTRAP_ROLLBACK(){
	
	local i=$TRAP_STOP

	LOG "Starting rollback. Start index - $TRAP_START"

	while [ $i -ge $TRAP_START ] ; do
		LOG "Rollback: ${TRAP_TRACK[$i]}"
		eval "${TRAP_TRACK[$i]}"
		LOG "rollback result" $?
		i=$(($i-1))
	done # while [ $i -ge $TRIP_START ] ; do
	
	TRAP_STOP=$(($TRAP_STOP+1))
	TRAP_START=$TRAP_STOP
	
} # SIGTRAP_ROLLBACK(){

function SIGTRAP_CLEAR(){
	
	LOG "Clean up rollback queue. Stop: $TRAP_STOP"

	TRAP_STOP=$(($TRAP_STOP+1))
	TRAP_START=$TRAP_STOP

} # SIGTRAP_CLEAR(){

function CHECK_COMMANDS(){
        [ "x$(type -p sed 2>&1)" != "x" ] \
            || DIE "Unable to find 'sed' command, please add its location to your PATH."

        local CHECK_CMD=$(echo "grep cp chmod uniq id sed mkdir rmdir readlink mktemp basename date cpio find wc cat tac ls gunzip rpm egrep tr cut uname" | sed -e"s/rpm //g")
        local c=''
        for c in ${CHECK_CMD} ; do
	    type -p $c &>/dev/null
	    if [ $? -ne 0 ] ; then
                if [ -f "/etc/mvl-release" ] && [ "$c" == "chkconfig" ] ; then
                : # there is no chkconfig command on MontaVista* CGE Linux*
                else
		    echo "ERROR: unable to find command '$c'."
		    echo "Please add the location to the above commands to your PATH and re-run the script"
		    echo -n "Press Enter to continue."
		    WAIT_ENTER_KEY
		    exit 1
                fi
	    fi
	done
}

function NONRPM_INSTALL_PACKAGE() {

    local rpm_path="$1"     # A path to rpm file to be installed.
    local dst_dir="$2"      # A path to directory to install package to.
    local edit_uninstall_mode=$3	# A mode to fix uninstall script for overwrite multiply installation
    [ "x$edit_uninstall_mode" == "x" ] && edit_uninstall_mode=1 	# To fix as per default
    local touch_dest=$4
    [ "x$touch_dest" == "x" ] && touch_dest=1
    local noscrits=$5
    local originaldir="$dst_dir"

    [[ -z "$dst_dir" ]] && dst_dir=$("$RPM_EXTR_TOOL" -qp --qf %{PREFIXES} "$rpm_path")
    dst_dir=$(echo "$dst_dir" | sed -e"s/\/\{1,\}/\//g")
    dst_dir=$(echo "$dst_dir" | sed -e"s/\/\{1,\}$//g")
    REL_TO_ABS "$dst_dir"
    rc=$?
    [ "x$rc" == "x0" ] && dst_dir="$RS"

    local db="$INTEL_SDP_PRODUCTS_DB"
    local db_dir=$(dirname "$db")

    local cur_dir=$(dirname "$rpm_path")
    REL_TO_ABS "$cur_dir"
    rc=$?
    [ "x$rc" == "x0" ] && cur_dir="$RS"

    local rpm_file=$(basename "$rpm_path")
    local rpm_prefix=$("$RPM_EXTR_TOOL" -qp --qf %{PREFIXES} "$rpm_path")
    local rpm_name=$("$RPM_EXTR_TOOL" -qp --qf %{NAME} "$rpm_path")
    local rpm_arch=$("$RPM_EXTR_TOOL" -qp --qf %{ARCH} "$rpm_path")
    local log_file=".$rpm_file"_$(date +'%d%m%y_%H%M%S').log
    local src_dir="$dst_dir/tmp12345qwexyz/${rpm_prefix}"

    if [ $touch_dest -eq 1 ]; then
	[ -e "$dst_dir" ] && rm -rf "$dst_dir" &>/dev/null
	[ -e "$dst_dir" ] || mkdir -p "$dst_dir" &>/dev/null
    fi

    local script_dir="$dst_dir/.scripts"
    mkdir -p "$script_dir"
    local script body
    for script in PREIN POSTIN PREUN POSTUN SUMMARY; do
        body=$("$RPM_EXTR_TOOL" -qp --qf "%{$script}" "$rpm_path")
        if [[ "$body" != "(none)" ]]; then
             echo "$body" > "$script_dir/$script.$rpm_name.$rpm_arch"
        fi
    done

    if [ "$noscripts" != "1" ]; then
        if [[ -e "$script_dir/PREIN.$rpm_name.$rpm_arch" ]]; then
    	    env RPM_INSTALL_PREFIX="${originaldir}" /bin/bash "$script_dir/PREIN.$rpm_name.$rpm_arch"
        fi
    fi

    mkdir "$dst_dir/tmp12345qwexyz" &>/dev/null
    cd "$dst_dir/tmp12345qwexyz"

    "$RPM_EXTR_TOOL" "$cur_dir/$rpm_file" | gunzip --quiet | cpio --quiet -idmu \
        || DIE "Unable to extract files from \"$rpm_file\" to temp filebuf."

    if [ $edit_uninstall_mode -eq 1 ]; then
		find "$dst_dir/tmp12345qwexyz/$rpm_prefix" -name uninstall*.sh > uninstall.lst
		local -a lines=( $( wc -l uninstall.lst ) )
		[ "${lines[0]}" -eq "1" ]
		rm -f lines.tmp
		local uninstall=$(cat "uninstall.lst")
		cp -p "$uninstall" "uninstall.sh.bak"
		chmod u+w "$uninstall"
		sed s@'^RPM_INSTALLATION=1$'@'RPM_INSTALLATION='@g "uninstall.sh.bak" > "$uninstall"
		chmod --reference="uninstall.sh.bak" "$uninstall"
		rm -f "uninstall.sh.bak" &>/dev/null
		rm -f "uninstall.lst" &>/dev/null
    fi

    "$RPM_EXTR_TOOL" "$cur_dir/$rpm_file" | gunzip --quiet | cpio --quiet -t | sed s@"^\.$rpm_prefix"@"$dst_dir"@g | sed 's/^\.//' > "$dst_dir/tmp12345qwexyz/$log_file" \
        || DIE "Unable to create log file."
    mkdir -p "$db_dir"
    mv "$dst_dir/tmp12345qwexyz/$log_file" "$db_dir"
    NONRPM_DB_ENTRY_ADD "<:$rpm_name:$rpm_file:$dst_dir:$db_dir/$log_file:>" \
        || DIE "Cannot add entry to database."
    local list=$(find "$dst_dir/tmp12345qwexyz" -type d | sed 's/^.*tmp12345qwexyz//' | sed s@"^$rpm_prefix"@"$dst_dir"@g | sed 's/^\.//')
    for one in $list; do mkdir -p "$one" &>/dev/null; done

    list=$(find "$dst_dir/tmp12345qwexyz")
    for one in $list; do
        if [ ! -L $one ] &>/dev/null ; then
	    if [ -d $one ] &>/dev/null ; then
	        continue
	    fi
	fi
        mv -f $one $(echo $one | sed 's/^.*tmp12345qwexyz//g' | sed s@"^$rpm_prefix"@"$dst_dir"@g | sed 's/^\.//') &>/dev/null
    done
    
    rm -rf "$dst_dir/tmp12345qwexyz"
    cd "$cur_dir"

    if [ "$noscripts" != "1" ]; then
        if [[ -e "$script_dir/POSTIN.$rpm_name.$rpm_arch" ]]; then
    	    env RPM_INSTALL_PREFIX="${originaldir}" /bin/bash "$script_dir/POSTIN.$rpm_name.$rpm_arch"
        fi
    fi

    return ${ERR_OK}
} # NONRPM_INSTALL_PACKAGE

function INSTALL_RPM(){

	local prefix="$3"
	local ChosenRPM="$1"
	shift
	local options="$1"
	local fcode=$ERR_OK
	
	LOG "Install rpm - $ChosenRPM"
	LOG "Options are: '$options'."

	
	RPM_CONFIG
	
	local opt_2=
	local prefix=`echo "$prefix" | sed s@--prefix[[:space:]]*@@`
	
	if echo $prefix | egrep "^\"" &> /dev/null ; then
		prefix=`echo "$prefix" | cut -d'"' -f2`
		opt_2=`echo "$options" | sed s@--prefix[[:space:]]*"$prefix"@@`
	else
		prefix=`echo "$prefix" | cut -d' ' -f1`
		opt_2=`echo "$options" | sed s@--prefix[[:space:]]*$prefix@@`
	fi
	
	
	local rpm_name=`RPM_INFO "$ChosenRPM" NAME`
	fcode=$?
	
	SIGTRAP_TRACK_ACTION UNINSTALL_RPM "$rpm_name"

	if [ $fcode -ne ${ERR_OK} ]; then
		return $fcode
	fi # if [ $? -ne ${ERR_OK} ]; then
    
    
    
	MAKE_TEMP_FILE
	local TEMPFILE="$RS"
	local err=$?
	[ $err -eq ${ERR_OK} ] || return $err
	
	if [ ${RPM_CONFIGURED} -eq ${ERR_RPM_NOT_RELOCATABLE} ] || [ "x$prefix" = "x" ]; then
		LOG "Using rpm install without relocation."
		rpm $opt_2 -vv $ChosenRPM &> $TEMPFILE
		err=$?
	else
		LOG "Using rpm install with relocation to '$prefix'"
		rpm $opt_2 -vv --prefix "$prefix" "$ChosenRPM" &> $TEMPFILE 
		err=$?
	fi # if [ ${RPM_CONFIGURED} -eq ${ERR_RPM_NOT_RELOCATABLE} ] ; then
	
	if [ $err -ne 0 ] ; then 
		fcode=$ERR_RPM_INSTALL
		local logs=$(sed 's/^/    /g' $TEMPFILE)
		LOG "Installation of $ChosenRPM. Logs are: "
		LOG "\n${logs}"
    	else
		LOG "'$ChosenRPM' installed"
	fi #if [ $err -ne 0 ] ; then
	
	rm -f "$TEMPFILE" &> /dev/null

	return $fcode
} #INSTALL_RPM(){

function USER_ID() {

    if [[ "$_USER_ID_" == "" ]]; then
        _USER_ID_=$( id -u ) || DIE "\"id\" failed."
        LOG "User id is \"$_USER_ID_\"."
    fi
    RS="$_USER_ID_"

} # function user_id

function TOOLS_CHECK(){
    ARC_GET

    local mydirname="$EXECDIR"
    REL_TO_ABS "$mydirname"
    rc=$?
    [ "x$rc" == "x0" ] && mydirname="$RS"
    case "$PLATFORM" in
    "x32")
	 RPM_EXTR_TOOL="$mydirname/./chklic/rpm_extract.32"
         ;;
    "x32_64")
         RPM_EXTR_TOOL="$mydirname/./chklic/rpm_extract.32e"
         ;;
    *)    
         DIE "Unable to detect HW platform (i*86, ia64 or i86_64) to prepare install enviroment or ia64 unsupported platform detected"
         ;;
    esac
    
    if [ -f "$RPM_EXTR_TOOL" ] ; then
	[ -x "$RPM_EXTR_TOOL" ] || DIE "NONRPM install tool has been corrupted. Installation aborted."
    else 
        DIE "NONRPM install tool has been corrupted. Installation aborted."
    fi
}

function RPM_ERRORHANDLER()
{
    cat $TEMPFILE.rpmout | sed 's@package .* is already installed@This was already installed.@g'
    QUIET=$1
    RPM_LOGS=$(cat $TEMPFILE.rpmout)
    if [ "$QUIET" != "y" ] ; then
        echo "Installation failed." ; echo
	echo "--------------------------------------------------------------------------------" ; echo
    fi
    rm -f $TEMPFILE.rpmout &> /dev/null
    rmdir $TEMPDIR &> /dev/null
}

function IS_INSTALLED()
{
    local package="$1"
    local product
    local rc=1

    local mydirname="$EXECDIR"
    REL_TO_ABS "$mydirname"
    rc=$?
    [ "x$rc" == "x0" ] && mydirname="$RS"
    rc=1
    if [ -n "$RPM_INSTALLATION" ]; then
	if [ -f "$mydirname/./rpms/$package" ] ; then
	    product=$(rpm -qp "$mydirname/./rpms/$package")
	    localarch=$(rpm -qp --qf %{ARCH} "$mydirname/./rpms/$package")
	else
	    product=$(echo $package | sed -e"s/\.rpm$//g" -e"s/\.[a-z0-9_]*$//g")
	    localarch=$(echo $package | sed -e"s/\.rpm$//g" -e"s/^.*\.\([a-z0-9_]*\)$/\1/g")
#"	
	fi

	rpm -q "$product" &>/dev/null
	rc=$?
	if [ "$rc" == "0" ] ; then
	    if [ "x$(rpm -q --qf %{ARCH} $product | grep $localarch)" == "x" ] ; then
    		rc=1
	    else
		rc=0
	    fi
	fi
    fi
    if [ "x$rc" != "x0" ] ; then
        NONRPM_DB_IS_PACKAGE_INSTALLED "$package"
	rc=$?	    
    fi
    return $rc
}

function INSTALL_PRODUCT() {
    echo "Installing package..."
    local mydirname="$EXECDIR"
    REL_TO_ABS "$mydirname"
    rc=$?
    [ "x$rc" == "x0" ] && mydirname="$RS"
    for one in $RPM_PACKAGES; do
	echo "Installing the $mydirname/../rpms/$one ..."
	if IS_INSTALLED "$one"; then
	    echo "The $mydirname/../rpms/$one has already been installed. Skip it ..."
	    continue
	fi
	if [ -n "$RPM_INSTALLATION" ]; then
	    INSTALL_RPM "$mydirname/../rpms/$one" "$DEFAULTOPTIONS" "--prefix $INSTALL_DESTINATION" || return 1
	else
	    if [ "x$(echo $one | grep noarch 2>/dev/null)" == "x" ]; then
    		NONRPM_INSTALL_PACKAGE "$mydirname/../rpms/$one" "$INSTALL_DESTINATION" 0 0 || return 1
	    else
		echo "Will create a NONRPM install directory, since this is noarch shared RPM ..."
		NONRPM_INSTALL_PACKAGE "$mydirname/../rpms/$one" "$INSTALL_DESTINATION" 0 0 || return 1
	    fi
	fi
    done

    return 0
} # function INSTALL_PRODUCT

declare CONFIG_PREFIX='SILENT'
declare CONFIG_SECTION='CPRO'
declare IC_IA32_RPM_PACKAGES="intel-openmp-117-13.0-1.i486.rpm
intel-openmp-devel-117-13.0-1.i486.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-compilerpro-vars-117-13.0-1.noarch.rpm
intel-compilerpro-devel-117-13.0-1.i486.rpm
intel-compilerproc-common-117-13.0-1.noarch.rpm
intel-compilerproc-devel-117-13.0-1.i486.rpm
intel-sourcechecker-common-117-13.0-1.noarch.rpm
intel-sourcechecker-devel-117-13.0-1.i486.rpm
intel-compilerproc-117-13.0-1.i486.rpm
"
declare IC_INTEL64_RPM_PACKAGES="intel-openmp-117-13.0-1.x86_64.rpm
intel-openmp-devel-117-13.0-1.x86_64.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-compilerpro-vars-117-13.0-1.noarch.rpm
intel-compilerpro-devel-117-13.0-1.x86_64.rpm
intel-compilerproc-common-117-13.0-1.noarch.rpm
intel-compilerproc-devel-117-13.0-1.x86_64.rpm
intel-sourcechecker-common-117-13.0-1.noarch.rpm
intel-sourcechecker-devel-117-13.0-1.x86_64.rpm
intel-compilerproc-117-13.0-1.x86_64.rpm
"
declare IF_IA32_RPM_PACKAGES="intel-openmp-117-13.0-1.i486.rpm
intel-openmp-devel-117-13.0-1.i486.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-compilerpro-vars-117-13.0-1.noarch.rpm
intel-compilerpro-devel-117-13.0-1.i486.rpm
intel-compilerprof-common-117-13.0-1.noarch.rpm
intel-compilerprof-117-13.0-1.i486.rpm
intel-compilerprof-devel-117-13.0-1.i486.rpm
intel-sourcechecker-common-117-13.0-1.noarch.rpm
intel-sourcechecker-devel-117-13.0-1.i486.rpm
"
declare IF_INTEL64_RPM_PACKAGES="intel-openmp-117-13.0-1.x86_64.rpm
intel-openmp-devel-117-13.0-1.x86_64.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-compilerpro-vars-117-13.0-1.noarch.rpm
intel-compilerpro-devel-117-13.0-1.x86_64.rpm
intel-compilerprof-common-117-13.0-1.noarch.rpm
intel-compilerprof-117-13.0-1.x86_64.rpm
intel-compilerprof-devel-117-13.0-1.x86_64.rpm
intel-sourcechecker-common-117-13.0-1.noarch.rpm
intel-sourcechecker-devel-117-13.0-1.x86_64.rpm
"
declare IDB_IA32_RPM_PACKAGES="intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-idb-common-117-13.0-1.noarch.rpm
intel-idbcdt-117-13.0-1.noarch.rpm
intel-idb-117-13.0-1.i486.rpm
"
declare IDB_INTEL64_RPM_PACKAGES="intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-idb-common-117-13.0-1.noarch.rpm
intel-idbcdt-117-13.0-1.noarch.rpm
intel-idb-117-13.0-1.x86_64.rpm
"
declare MKL_IA32_RPM_PACKAGES="intel-openmp-117-13.0-1.i486.rpm
intel-openmp-devel-117-13.0-1.i486.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-mkl-common-117-11.0-1.noarch.rpm
intel-mkl-117-11.0-1.i486.rpm
intel-mkl-devel-117-11.0-1.i486.rpm
"
declare MKL_INTEL64_RPM_PACKAGES="intel-openmp-117-13.0-1.x86_64.rpm
intel-openmp-devel-117-13.0-1.x86_64.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-mkl-common-117-11.0-1.noarch.rpm
intel-mkl-117-11.0-1.x86_64.rpm
intel-mkl-devel-117-11.0-1.x86_64.rpm
"
declare TBB_RPM_PACKAGES="intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-tbb-117-4.1-1.noarch.rpm
intel-tbb-devel-117-4.1-1.noarch.rpm
"

declare TBB_SOURCE_RPM_PACKAGES="intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-tbb-117-4.1-1.noarch.rpm
intel-tbb-devel-117-4.1-1.noarch.rpm
intel-tbb-source-117-4.1-1.noarch.rpm
"

declare IPP_IA32_RPM_PACKAGES="intel-openmp-117-13.0-1.i486.rpm
intel-openmp-devel-117-13.0-1.i486.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-ipp-common-117-7.1-1.noarch.rpm
intel-ipp-117-7.1-1.i486.rpm
intel-ipp-devel-117-7.1-1.i486.rpm
"
declare IPP_LP32_RPM_PACKAGES="intel-openmp-117-13.0-1.i486.rpm
intel-openmp-devel-117-13.0-1.i486.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-ipp-common-117-7.1-1.noarch.rpm
intel-ipp-117-7.1-1.i686.rpm
intel-ipp-devel-117-7.1-1.i686.rpm
"
declare IPP_INTEL64_RPM_PACKAGES="intel-openmp-117-13.0-1.x86_64.rpm
intel-openmp-devel-117-13.0-1.x86_64.rpm
intel-compilerpro-common-117-13.0-1.noarch.rpm
intel-ipp-common-117-7.1-1.noarch.rpm
intel-ipp-117-7.1-1.x86_64.rpm
intel-ipp-devel-117-7.1-1.x86_64.rpm
"

CHECK_COMMANDS
SAVE_COMMAND_LINE $@

IS_COMMAND_LINE_OPTION_EXISTS run || DIE "This script installs Intel(R) products and should be called from general install script. Exiting..."

SCRIPT="$0"
EXECDIR=$(dirname "$SCRIPT")
REL_TO_ABS "$EXECDIR"
rc=$?
[ "x$rc" == "x0" ] && EXECDIR="$RS"
DEFAULTOPTIONS='-ivh --force --nodeps --ignorearch --nomd5'

PATH="/sbin:/usr/sbin:/bin:/usr/bin/:/usr/local/sbin:/usr/local/bin:/usr/local/gnu/bin:${PATH}:"
export PATH

TOOLS_CHECK

IS_COMMAND_LINE_OPTION_EXISTS silent && SET_CONFIG_FILE
uid=$(id -u)
[ $? != 0 ] && uid=1
if [ $uid == 0 ]; then
    NONRPM_DB_PREFIX=/opt/intel
else
    NONRPM_DB_PREFIX="$HOME/intel"
fi
NONRPM_SET_DB_MODE

READ_DEFAULT_ANSWER "INSTALL_DESTINATION"
if [ "x$RS" == "x" ]; then
    echo "SILENT managing: INSTALL_DESTINATION has not been recognized ..."
    INSTALL_DESTINATION="/opt/intel/composer_xe_2013.1.117"
else
    INSTALL_DESTINATION="$RS"
fi
REL_TO_ABS "$INSTALL_DESTINATION"
if [ $? -eq 0 ]; then
    INSTALL_DESTINATION="$RS"
fi
echo "Will be installed to ${INSTALL_DESTINATION} ..."

READ_DEFAULT_ANSWER "INSTALL_MODE"
if [ "x$RS" == "x" ]; then
    echo "SILENT managing: INSTALL_MODE has not been recognized ..."
    INSTALL_MODE="RPM"
else
    INSTALL_MODE="$RS"
fi
echo "Install mode is ${INSTALL_MODE} ..."

if [ "x${INSTALL_MODE}" == "xRPM" ] ; then
    RPM_INSTALLATION=1
else
    RPM_INSTALLATION=
    SAVE_COMMAND_LINE "$@ --nonrpm"
fi

if IS_COMMAND_LINE_OPTION_EXISTS ic_ia32; then
    echo "IC IA32 key enabled ..."
    RPM_PACKAGES=$IC_IA32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS ic_intel64; then
    echo "IC INTEL64 key enabled ..."
    RPM_PACKAGES=$IC_INTEL64_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS idb_ia32; then
    echo "IDB IA32 key enabled ..."
    RPM_PACKAGES=$IDB_IA32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS idb_intel64; then
    echo "IDB INTEL64 key enabled ..."
    RPM_PACKAGES=$IDB_INTEL64_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS mkl_ia32; then
    echo "MKL IA32 key enabled ..."
    RPM_PACKAGES=$MKL_IA32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS mkl_intel64; then
    echo "MKL INTEL64 key enabled ..."
    RPM_PACKAGES=$MKL_INTEL64_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS ipp_ia32; then
    echo "IPP IA32 key enabled ..."
    RPM_PACKAGES=$IPP_IA32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS ipp_lp32; then
    echo "IPP LP32 key enabled ..."
    RPM_PACKAGES=$IPP_LP32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS ipp_intel64; then
    echo "IPP INTEL64 key enabled ..."
    RPM_PACKAGES=$IPP_INTEL64_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS tbb; then
    echo "TBB key enabled ..."
    RPM_PACKAGES=$TBB_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS tbb_source; then
    echo "TBB SOURCE key enabled ..."
    RPM_PACKAGES=$TBB_SOURCE_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS if_ia32; then
    echo "IF IA32 key enabled ..."
    RPM_PACKAGES=$IF_IA32_RPM_PACKAGES
fi
if IS_COMMAND_LINE_OPTION_EXISTS if_intel64; then
    echo "IF INTEL64 key enabled ..."
    RPM_PACKAGES=$IF_INTEL64_RPM_PACKAGES
fi

[ -n "$RPM_INSTALLATION" ] && RPM_CONFIG
INSTALL_PRODUCT
if [ $? -ne 0 ]; then
    echo "Installation has been finished unsuccessfully."
    DIE "Installtion failed."
fi

SHARED_LOCATION=$(echo "$INSTALL_DESTINATION" | grep -e"\/composer_xe_2013\.[0-9]\.[0-9][0-9][0-9]$" | sed -e"s/\/composer_xe_2013\.[0-9]\.[0-9][0-9][0-9]$//g")
if IS_COMMAND_LINE_OPTION_EXISTS master ; then
    `${EXECDIR}/link_install.sh -u -l ${SHARED_LOCATION} &>/dev/null`
else
    `${EXECDIR}/link_install.sh -u -l ${SHARED_LOCATION} -m slave &>/dev/null`
fi

echo "Installation has been finished successfully."
exit 0
