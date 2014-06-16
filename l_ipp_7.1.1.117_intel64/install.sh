#!/bin/sh

# Copyright (c) 2006-2012 Intel Corporation. All rights reserved.
# This script installs Intel(R) Software Development Products.

touch_space(){
	
	local dir_to_check=$1
	local dir_end_path='hags7823782318#@123kjhknmnzxmnz'
	local err=0
	
	if [ -d "$dir_to_check" ] ; then 
		if [ -w "$dir_to_check" ] ; then
			mkdir "$dir_to_check/$dir_end_path" >/dev/null 2>&1
			err=$?
			if [ "$err" = "0" ] ; then
				rmdir "$dir_to_check/$dir_end_path" >/dev/null 2>&1
			fi
		else
			err=1
		fi
	else
		touch_space "`dirname $dir_to_check`" "$dir_end_path"
		err=$?
	fi
	
	return $err
}

check_free_space()
{
    location=$1
    need_space=$2
    real_location=$location
    while [ ! -d "$real_location" ] ; do
        real_location=$(dirname $real_location)
    done
    exist_space=$(df -Pm $real_location | tail -n1 | tr -s "[:space:]" | cut -d ' ' -f 4)
    if [ "$need_space" -gt "$exist_space" ]; then
    	echo "Error: not enough disk space in temporary folder \"$real_location.\""
    	echo "Available: ${exist_space}M"
    	echo "Required: ${need_space}M"
	echo ""
	echo "Quitting! Press \"Enter\" to terminate install."
	read DONTCARE
	exit 1        
    fi
}

check_cpu_model() {
    CPU_FAMILY=$(echo $(grep family /proc/cpuinfo |head -1|cut -d: -f2))
    CPU_MODEL=$(echo $(grep model /proc/cpuinfo |head -1|cut -d: -f2))

    if [ "ia64" = "$(uname -m)" ]; then
        return
    fi

    MIN_FAMILY=6
    MIN_MODEL=14

    if [ $CPU_FAMILY -lt $MIN_FAMILY ]; then
        echo "CPU is not supported."
        exit 1
    elif [ $CPU_FAMILY -eq $MIN_FAMILY ] && [ $CPU_MODEL -lt $MIN_MODEL ]; then
        echo "CPU is not supported."
        exit 1
    fi
}

prepare_pset_binary()
{
    # check for zip feature 
    if [ -f "$fullpath/pset/$pset_archive_name" ]; then
        old_structure_type="no"
    fi

    if [ "$old_structure_type" = "yes" ]; then
        check_free_space $user_tmp $require_tmp_space_org
        # check the platform support
	if [ ! -f "$fullpath/install.sh" ]; then
    	    echo "Can not execute $fullpath/pset/$my_arch/install.$my_arch: permission denied."
	    echo "Please check the package was unpacked with proper permissions."
	    echo ""
	    echo "Quitting! Press \"Enter\" to terminate install."
	    read DONTCARE
	    exit 1
	fi

        if [ ! -f "$fullpath/pset/$my_arch/install.$my_arch" ]; then
            if [ ! -d "$fullpath/pset/$my_arch" ]; then
		echo "The package does not support the platform it being run on."
		echo "Please check to be sure the correct architecture of package has been downloaded."
		echo ""
		echo "Quitting! Press \"Enter\" to terminate install."
		read DONTCARE
		exit 1            
            fi
            if [ ! -x "$fullpath/pset/$my_arch" ]; then
		echo "Can not execute $fullpath/pset/$my_arch/install.$my_arch: permission denied."
		echo "Please check the package was unpacked with proper permissions."
		echo ""
		echo "Quitting! Press \"Enter\" to terminate install."
		read DONTCARE
		exit 1
	    fi
	    
	    echo "The package does not support the platform it being run on."
	    echo "Please check to be sure the correct architecture of package has been downloaded."
	    echo ""
	    echo "Quitting! Press \"Enter\" to terminate install."
	    read DONTCARE
	    exit 1
	fi

        if [ ! -x "$fullpath/pset/$my_arch/install.$my_arch" ]; then
	    echo "Can not execute $fullpath/pset/$my_arch/install.$my_arch: permission denied."
	    echo "Please check the package was unpacked with proper permissions."
	    echo ""
	    echo "Quitting! Press \"Enter\" to terminate install."
	    read DONTCARE
	    exit 1
	fi
    
        if [ -z "${libstdc_exist}" ]; then
    	    libstdc_path="$fullpath/pset/$my_arch/gcc-3.2"
	fi
    
        export LD_LIBRARY_PATH="$fullpath/pset/$my_arch:$libstdc_path:$LD_LIBRARY_PATH"        
        cd $fullpath/pset/$my_arch >/dev/null 2>&1
        
        pset_engine_binary="$fullpath/pset/$my_arch/install.$my_arch"
        pset_engine_folder="$fullpath/pset/$my_arch"
    else
	echo "Extracting, please wait..."
        check_free_space $user_tmp $require_tmp_space_zip
        temp_dir="$user_tmp/intel.pset.$USER.${HOSTNAME}_$(date +%m.%d.%H.%M.%S.%Y)"
        mkdir -p $temp_dir
        tar zxf $fullpath/pset/$pset_archive_name -C $temp_dir >/dev/null 2>&1
    
        # check the platform support
        if [ ! -f "$temp_dir/$my_arch/install.$my_arch" ]; then
	    echo "The package does not support the platform it being run on."
	    echo "Please check to be sure the correct architecture of package"
	    echo "has been downloaded."
	    echo ""
	    echo "Quitting! Press \"Enter\" to terminate install."
	    read DONTCARE
	    exit 1
	fi

        if [ -z "${libstdc_exist}" ]; then
		libstdc_path="$temp_dir/$my_arch/gcc-3.2"
        fi

        export LD_LIBRARY_PATH="$temp_dir/$my_arch:$libstdc_path:$LD_LIBRARY_PATH"        
        cd $temp_dir/$my_arch >/dev/null 2>&1
	
        pset_engine_binary="$temp_dir/$my_arch/install.$my_arch"
        pset_engine_folder="$fullpath/pset/$my_arch"
    fi
}

parse_cmd_parameters()
{
    while [ $# -gt 0 ] ; do
    case "$1" in
	--silent|-s)
	    # silent install
	    if [ -z "$2" ]; then
		echo "Error: Please provide silent configuration file."
		exit 1
	    fi
	    exe=`basename $2 2>/dev/null`
	    dir=`dirname $2 2>/dev/null`
	    if echo $dir | grep -q -s ^/ || echo $dir | grep -q -s ^~ ; then
		# absolute path
		silent_params="--silent $dir/$exe"
		silent_cfg="$dir/$exe"
	    else
		# relative path 
		silent_params="--silent $runningdir/$dir/$exe"
		silent_cfg="$runningdir/$dir/$exe"
	    fi
	    if [ ! -f "$silent_cfg" ]; then
		echo "Error: \"$silent_cfg\" doesn't look like a proper silent configuration file."
		echo "Please make sure that this file exists and run installation again."
	        exit 1
	    fi
	    skip_uid_check="yes"
	    shift
	    ;;
	--duplicate|-d)
	    # duplicate install
	    if [ -z "$2" ]; then
		echo "Error: Please provide silent configuration file."
		exit 1
	    fi
	    exe=`basename $2 2>/dev/null`
	    dir=`dirname $2 2>/dev/null`
	    if echo $dir | grep -q -s ^/ || echo $dir | grep -q -s ^~ ; then
		# absolute path
		duplicate_params="--duplicate $dir/$exe"
		duplicate_cfg="$dir/$exe"
	    else
		# relative path 
		duplicate_params="--duplicate $runningdir/$dir/$exe"
		duplicate_cfg="$runningdir/$dir/$exe"
	    fi
	    shift	
	    ;;
	--help|-h)
	    # show help message
	    params="$params -h"
	    skip_uid_check="yes"
	    skip_cd_eject="yes"
	    skip_selinux_check="yes"
	    break
	    ;;
	--version|-v)
	    # show version info
	    params="$params --__version__"
	    skip_uid_check="yes"
	    skip_cd_eject="yes"
	    skip_selinux_check="yes"
	    break
	    ;;
	--lang|-l)
	    # user set lang
	    user_lang="$2"
	    params="$params --LANG=$2"
	    shift
	    ;;
	--user-mode)
	    # run installation under current user privileges
	    skip_uid_check="yes"
	    ;;
    --ignore-cpu)
	    # skip cpu checking
        skip_cpu_check="yes"
        ;;
	--tmp-dir|-t)
	    if [ -z "$2" ]; then
		echo "Error: Please provide temporal folder."
		exit 1
	    fi
	    user_tmp="$2"
	    if [ ! -d "$user_tmp" ]; then
		echo "Error: $user_tmp doesn't look like a proper folder."
		echo "Please make sure that this folder exists and run installation again."
		echo ""
		echo "Quitting! Press \"Enter\" to terminate install."
		read DONTCARE
	        exit 1
	    fi

	    #params="$params --tmp-dir=$2"
	    shift
	    ;;
	*)
	    params="$params $1"
	    #check for LANG option
	    is_lang=$(echo $1 | grep "LANG")
	    if [ ! -z "$is_lang" ]; then
		user_lang=$(echo $1 | cut -d= -f2)
	    fi
	    ;;
	esac
	shift
    done
}

check_runningdir()
{
    if [ -n "$(echo "$fullpath" | grep " ")" ] ; then
	echo "Error: Incorrect path to installation script. Installation can not be started"
	echo "if the path contains space symbols."
	echo ""
	echo "Quitting! Press \"Enter\" to terminate install."
	read DONTCARE
        exit 1
    fi

    if [ -n "$(echo "$fullpath" | egrep -e ':' -e '~' -e '&' -e '%' -e '#' -e '@' -e '\[' -e '\]' -e '\$' -e '=' -e '\)' -e '\(' -e '\*')" ] ; then
	echo "Error: Incorrect path to installation script. Installation can not be started"
	echo "if the path contains ':, ~, @, #, %, &, [, ], $, =, ), (, *' symbols."
	echo ""
	echo "Quitting! Press \"Enter\" to terminate install."
	read DONTCARE
        exit 1
    fi
}

# script start
thisexec=`basename $0`
thisdir=`dirname $0`
runningdir=`pwd`
[ -z "$HOSTNAME" ] && HOSTNAME=$(hostname);
log_file=intel.pset.$USER.${HOSTNAME}_$(date +%m.%d.%H.%M.%S.%Y).log
old_structure_type="yes"
pset_archive_name="pset.tgz"
strings_list="bash_root_nonroot;bash_root_nonroot_question;bash_log_as_root;bash_log_as_root_failed;bash_quit;bash_log_as_sudo;bash_log_as_sudo_failed;bash_log_as_user;bash_log_help;bash_log_quit;bash_log_invalid_choice;to_continue_question;bash_selinux_error;bash_pset_already_run"
strings_file=intel.pset.strings.$USER.${HOSTNAME}
strings_log_file=intel.pset.strings.$USER.${HOSTNAME}.log
require_tmp_space_zip=100
require_tmp_space_org=5
pset_marker=intel.pset.$USER.running.marker

trap "" TSTP # Disable Ctrl-Z

# detect bash script source execution
if [ -z "$(echo "$0" | grep "install.sh")" ]; then
    echo "Script is running sourced ..."
    echo "ERROR: This script installs product and should be called directly. Exiting..."
    exit 1
fi

if echo $thisdir | grep -q -s ^/ || echo $thisdir | grep -q -s ^~ ; then
# absolute path
   fullpath="$thisdir"
else
# relative path 
   fullpath="$runningdir/$thisdir"
fi
check_runningdir

system_cpu=`uname -m`
if [ "$system_cpu" = "ia64" ]; then
    my_arch=64
elif [ "$system_cpu" = "x86_64" ]; then
    my_arch=32e
else
    my_arch=32
fi

if [ "$my_arch" = "32" ]; then
    libstdc_exist=`/sbin/ldconfig -p | grep libstdc++.so.5 | cut -d'>' -f2`
elif [ "$my_arch" = "32e" ]; then
    libstdc_exist=`/sbin/ldconfig -p | grep libstdc++.so.5 | grep x86-64 | cut -d'>' -f2`
else
    libstdc_exist=`/sbin/ldconfig -p | grep libstdc++.so.5 | grep IA-64 | cut -d'>' -f2`
fi

libstdc_exist="${libstdc_exist## }"
libstdc_exist="${libstdc_exist%% }"
if [ ! -f "$libstdc_exist" ]; then
    libstdc_exist=""
fi

unset libstdc_path
#umask 077
#parse parameters
params=""
parse_cmd_parameters $@

if [ "$skip_cpu_check" != "yes" ]; then
    check_cpu_model
fi

#set log files
if [ -z "$user_tmp" ]; then
    if [ -z "$TMPDIR" ]; then    
        user_tmp="/tmp"
    else
	if [ -d "$TMPDIR" ]; then
	    user_tmp=$TMPDIR
	else
	    user_tmp="/tmp"
	fi
    fi
fi
touch_space $user_tmp
check=$?
if [ "$check" != "0" ]; then
    echo "Error: No write permissions to \"$user_tmp\" temporary folder."
    echo "Please fix the permissions or provide another temporary folder."
    echo "To provide another temporary folder please run installation"
    echo "with \"--tmp-dir [FOLDER]\" parameter."
    echo ""
    echo "Quitting! Press \"Enter\" to terminate install."
    read DONTCARE
    exit 1
fi

log_file="$user_tmp/$log_file"
strings_file="$user_tmp/$strings_file"
strings_log_file="$user_tmp/$strings_log_file"
#get l10n strings
prepare_pset_binary
if [ -z "$user_lang" ]; then
    if [ ! -z "$LC_CTYPE" ]; then
	user_lang="$LC_CTYPE"
    elif [ ! -z "$LANG"  ]; then
	user_lang="$LANG"
    else
	user_lang="default"
    fi
fi
$pset_engine_binary --tmp-dir=$user_tmp --log-file=$strings_log_file --string_ids="$strings_list" --__get_string__=$strings_file --LANG=$user_lang
. $strings_file
rm $strings_file >/dev/null 2>&1
rm $strings_log_file >/dev/null 2>&1

# check already running PSET
IS_PSET_MARKER="no"
if [ -f "$user_tmp/$pset_marker" ]; then
    IS_PSET_MARKER="yes"
fi

IS_PSET_PROCESS="no"
if pgrep install.$my_arch >/dev/null; then
    IS_PSET_PROCESS="yes"
fi

if [ "$IS_PSET_MARKER" = "yes" ] ; then
    if [ "$IS_PSET_PROCESS" = "yes" ] ; then
	echo "$LI_bash_pset_already_run"
        exit 1
    fi
    # remove marker
    rm -f "$user_tmp/$pset_marker" >/dev/null 2>&1
fi

# check selinux
if [ "$skip_selinux_check" != "yes" ]; then
    SELINUXENABLED_CMD=`which selinuxenabled 2>/dev/null`
    if [ -z "$SELINUXENABLED_CMD" ] ; then
	SELINUX_PATH="/etc/sysconfig/selinux"
        if [ -e "$SELINUX_PATH" ] ; then
	    [ -z `cat "$SELINUX_PATH" | grep "SELINUX=disabled"` ] && SELINUXENABLED="y"
        fi
    else
	$SELINUXENABLED_CMD
        [ $? -eq 0 ] && SELINUXENABLED="y"
    fi

    if [ "$SELINUXENABLED" = "y" ] ; then
        if [ "off" = "$(getsebool allow_execmod | cut -d' ' -f3)" ] ||
           [ "off" = "$(getsebool allow_execstack | cut -d' ' -f3)" ]; then
        echo "$LI_bash_selinux_error"
        echo ""
    	    echo "$LI_bash_quit"
            read 
            exit 0  
        fi
    fi
fi
#check for valid silent config file
if [ ! -z "$silent_params" ]; then # need to validate silent config file
    awk -f "$fullpath/pset/check.awk" $silent_cfg
    exit_code=$?
    if [ $exit_code != "0" ]; then
	exit 1
    fi
fi

[ -z "$UID" ] && UID=$(id -ru);

if [ "$UID" -ne 0 ]; then

    if [ "$skip_uid_check" != "yes" ]; then
	REPEAT_LOOP=1
    else
	REPEAT_LOOP=0
    fi
 
 
    while [ "$REPEAT_LOOP" = 1 ]; do
	echo "$LI_bash_root_nonroot"    
	echo " "
        echo -n "$LI_bash_root_nonroot_question"
        read  usr_choice >/dev/null 2>&1
        if [ -z "$usr_choice" ]; then
    	    usr_choice=1
        fi
        case $usr_choice in
        1 )
                echo "$LI_bash_log_as_root"
		subcommand="sh $fullpath/$thisexec $silent_params $duplicate_params $params --LANG=$user_lang || true"
 		sh -c "(exec su - -c \"$subcommand\")"
		if [ "$?" = "0" ]; then
			if [ "$old_structure_type" = "no" ]; then
				rm -rf $temp_dir >/dev/null 2>&1
			fi
			exit 0;
		else 
			echo -n "$LI_bash_log_as_root_failed"
			read usr_choice >/dev/null 2>&1
			if [ "$usr_choice" = "y" ] || [ -z "$usr_choice" ]; then 
				REPEAT_LOOP=1
			else
				echo "$LI_bash_quit"
				read DONTCARE
				if [ "$old_structure_type" = "no" ]; then
					rm -rf $temp_dir >/dev/null 2>&1
				fi
				exit 0
			fi	
		fi
                ;;
        2 )
                echo "$LI_bash_log_as_sudo"
		subcommand="sh $fullpath/$thisexec $silent_params $duplicate_params $params --LANG=$user_lang || true"
  		sh -c "(sudo su - -c \"$subcommand\")"
		if [ "$?" = "0" ]; then
			if [ "$old_structure_type" = "no" ]; then
				rm -rf $temp_dir >/dev/null 2>&1
			fi
			exit 0
		else
			echo -n "$LI_bash_log_as_sudo_failed"
			read usr_choice >/dev/null 2>&1
			if [ "$usr_choice" = "y" ] || [ -z "$usr_choice" ]; then 
				REPEAT_LOOP=1
			else
				echo "$LI_bash_quit"
				read DONTCARE
				if [ "$old_structure_type" = "no" ]; then
					rm -rf $temp_dir >/dev/null 2>&1
				fi
				exit 0
			fi	
		fi
                ;;
	3 )
		echo "$LI_bash_log_as_user"
		REPEAT_LOOP=0
		;;

        h )
                echo "$LI_bash_log_help"
                echo -n "$LI_to_continue_question"
                read  dummy >/dev/null 2>&1

                ;;
        q )
                echo "$LI_bash_log_quit"
                if [ "$old_structure_type" = "no" ]; then
		    rm -rf $temp_dir >/dev/null 2>&1
		fi

                exit ;;

        * ) echo "$LI_bash_log_invalid_choice"
                 REPEAT_LOOP=1 ;;


        esac
    done
fi

touch "$user_tmp/$pset_marker" >/dev/null 2>&1
$pset_engine_binary --tmp-dir=$user_tmp --log-file=$log_file $silent_params $duplicate_params $params --PACKAGE_DIR=$fullpath
exit_code=$?
rm -f "$user_tmp/$pset_marker" >/dev/null 2>&1
cd - >/dev/null 2>&1

if [ "$old_structure_type" = "no" ]; then
    rm -rf $temp_dir >/dev/null 2>&1
fi

## CD Eject Issue

if [ -f "$fullpath"/cd_eject.sh ]; then
    if [ -z "$skip_cd_eject" ]; then
	"$fullpath"/cd_eject.sh $PPID
        exit $exit_code
    fi
fi
exit $exit_code
