#!/bin/bash
# Copyright 2010-2012, Intel Corporation, All Rights Reserved.
# link_install.sh - makes symbolic links and other environment changes as 
# part of an Intel(R) Composer XE 2013 for Linux* installation
#
# -----------------------------------------------------------------------------
# NOTE: packagid must be initialized by build system for proper operation.
#       See line(s) starting with "PACKAGEID="

# -----------------------------------------------------------------------------
# create a directory
# return non-ZERO if failure
ICP12_CREATE_DIR() {
    local THIS_DIR="$1"
    if [ -d "$THIS_DIR" ]; then
        VERBOSE_PRINT "dir <$THIS_DIR> already exists.  Not created."
        return 0
    fi
    VERBOSE_PRINT "Making dir <$THIS_DIR> ..."
    mkdir "$THIS_DIR" 2>/dev/null
    if [ $? -ne 0 ]; then
        VERBOSE_PRINT "Problems creating directory <$THIS_DIR>"
        return 1
    fi
}
# -----------------------------------------------------------------------------
# create a symbolic link to a directory from an existing directory
# if there is a pre-existing one, remove it first
# return non-ZERO if failure
# - create link from THIS_LINK_POINT_DIR to THIS_NEW_LINK_DIR
ICP12_CREATE_DIR_LINK() {
    local THIS_LINK_POINT_DIR="$1"
    local THIS_NEW_LINK_DIR="$2"
    local IS_REWRITABLE="$3"

    if [ -L $THIS_NEW_LINK_DIR ]; then
        if  [ "$IS_REWRITABLE" = "y" ]; then
            VERBOSE_PRINT "Removing file or link $THIS_NEW_LINK_DIR ..."
            rm $RM_OPTIONS $THIS_NEW_LINK_DIR 2>/dev/null
        else
            VERBOSE_PRINT "Directory $THIS_NEW_LINK_DIR exists, but we're in SLAVE mode... doing nothing"
            return 0
        fi
    else
        # if we have old mkl/ipp/tbb installed (such directories exist in /opt/intel), then we
        # should not create links
        if [ -d $THIS_NEW_LINK_DIR ]; then
            return 0
        fi
    fi

    # THIS_LINK_POINT_DIR can be relative, so we need to check its absolute one
    local ABS_LINK_POINT_DIR="$(dirname $THIS_NEW_LINK_DIR 2>/dev/null)/$THIS_LINK_POINT_DIR"
    if [ ! -d "$ABS_LINK_POINT_DIR" ]; then
        echo "warning: directory $ABS_LINK_POINT_DIR isn't exist for current package $PACKAGEID -  "
        # if link point is PACKAGE dependendent, trying to find it in another package
        if [ "x$(echo $THIS_LINK_POINT_DIR | grep $PACKAGEID)" != "x" ]; then
            echo "...trying to find the latest package for this component..."
            CHECK_LATEST_VER_WHERE_FILE_PRESENT "$(echo $THIS_LINK_POINT_DIR | sed "s/^.*$PACKAGEID//g")" "-d"
            if [ $? -eq 0 ]; then
                echo "......found! package is $REPLY_VERSION"
                THIS_LINK_POINT_DIR=$(echo $THIS_LINK_POINT_DIR | sed "s/$PACKAGEID/composer_xe_$REPLY_VERSION/g")
            else
                echo "......no installed versions found containing such component"
                return 1
            fi
        else
            echo "a link to this directory will not be created (it's not package dependent)"
            return 1
        fi
    fi

    VERBOSE_PRINT "creating dir link from <$THIS_LINK_POINT_DIR> to <$THIS_NEW_LINK_DIR>"
    ln -s $THIS_LINK_POINT_DIR $THIS_NEW_LINK_DIR 2>/dev/null
    if [ ! -L "$THIS_NEW_LINK_DIR" ]; then
        VERBOSE_PRINT "Problems creating link $THIS_NEW_LINK_DIR to $THIS_LINK_POINT_DIR"
        popd &>/dev/null
    fi
}

# -----------------------------------------------------------------------------
# create a symbolic link to a file
# if there is a pre-existing one, remove it first
# return non-ZERO if failure
# - create link FROM THIS_EXIST_FILE TO THIS_LINK_FILE
ICP12_CREATE_FILE_LINK() {
    local THIS_EXIST_FILE="$1"
    local THIS_LINK_FILE="$2"

    if [ -L "$THIS_LINK_FILE" ]; then
        VERBOSE_PRINT "Link '$THIS_LINK_FILE' exists. Removing it prior setting a new one"
        rm ${RM_OPTIONS} ${THIS_LINK_FILE} &>/dev/null
    fi

    VERBOSE_PRINT "trying to create link $THIS_LINK_FILE"
    if [ ! -f "$THIS_EXIST_FILE" ]; then
        echo "warning: file $THIS_EXIST_FILE isn't exist - a link to this file won't be created"
        return 1
    fi
    VERBOSE_PRINT "creating file link from <$THIS_EXIST_FILE> to <$THIS_LINK_FILE>"
    ln -s $THIS_EXIST_FILE $THIS_LINK_FILE 2>/dev/null
    if [ ! -L "$THIS_LINK_FILE" ]; then
        VERBOSE_PRINT "Problems creating link $THIS_LINK_FILE to $THIS_EXIST_FILE"
        return 1
    fi
}
# -----------------------------------------------------------------------------
# copy a file (create a real, non-link copy of the file)
# return non-ZERO if failure
ICP12_COPY_FILE() {
    local THIS_EXIST_FILE="$1"
    local THIS_NEW_FILE="$2"
    if [ -e "$THIS_EXIST_FILE" ]; then
        VERBOSE_PRINT "copying from <$THIS_EXIST_FILE> to <$THIS_NEW_FILE>"
        cp -p $THIS_EXIST_FILE $THIS_NEW_FILE &>/dev/null
    else
        echo "Problems copying file $THIS_EXIST_FILE to $THIS_NEW_FILE"
        echo "Existing file $THIS_EXIST_FILE not found"
        return 1
    fi
    if [ ! -e "$THIS_NEW_FILE" ]; then
        echo "Problems copying file $THIS_EXIST_FILE to $THIS_NEW_FILE"
        return 1
    fi
}
# -----------------------------------------------------------------------------
# detect current state of Intel Compiler Professional 12.0 installs, if any
# - this should be executed AFTER ICP12_INIT
ICP12_DETECT() {
    VERBOSE_PRINT "ICP12_DETECT_MKLINKS()"

    # if PKG_INSTALL_DESTINATION is empty, this is an error
    if [ -z "$PKG_INSTALL_DESTINATION" ]; then
        PKG_INSTALL_DESTINATION_EXISTS=0
        echo "PKG_INSTALL_DESTINATION is empty or undefined."
        echo "Internal error"
        exit 1
    fi

    # see if package installation directory exists
    # if not exist, this is also an error
    if [ -d "$PKG_INSTALL_DESTINATION" ]; then
        PKG_INSTALL_DESTINATION_EXISTS=1
    else
        echo "PKG_INSTALL_DESTINATION $PKG_INSTALL_DESTINATION does not exist."
        echo "Please make sure that product is installed to specified location ($PKG_INSTALL_DESTINATION)"
        exit 1
    fi

    local str=''

    for str in $LINK_STRUCT; do
        local DIR_VAR=$(_get_dir_var "$str"; echo $REPLY)
        local DIR_PATH=$(_get_dir_path "$str"; echo $REPLY)
        if [ -z "$DIR_VAR" ] || [ -z "$DIR_PATH" ]; then
            echo "warning: internal error while parsing '$str' string"
        fi
        if [ -d "$DIR_PATH" ]; then
            eval ${DIR_VAR}_EXISTS=1
        else
            eval ${DIR_VAR}_EXISTS=0
        fi
    done
}

# -----------------------------------------------------------------------------
# report status
ICP12_REPORT() {
    echo "ICP12_REPORT()"

    local str=''

    for str in $LINK_STRUCT; do
        local DIR_VAR=$(_get_dir_var "$str"; echo $REPLY)
        local DIR_PATH=$(_get_dir_path "$str"; echo $REPLY)
        if [ -z "$DIR_VAR" ] || [ -z "$DIR_PATH" ]; then
            echo "warning: internal error while parsing '$str' string"
        fi
        echo -n "$DIR_VAR=$DIR_PATH"
        if [ "x$(eval echo \${${DIR_VAR}_EXISTS})" = "x1" ]; then
            echo " (exists)"
        else
            echo " (not present)"
        fi
    done
}

# -----------------------------------------------------------------------------
# create necessary links
ICP12_MKLINKS() {
    VERBOSE_PRINT "ICP12_MKLINKS()"
    
    SET_ARCH

    # Intel TOP Directory - create if not exist, exit in case of error
    # NOTE: we should not create links structure in other than /opt/intel dir
    if [ "$TOP_INTEL_DIR_EXISTS" = "0" ]; then
        ICP12_CREATE_DIR $TOP_INTEL_DIR
        if [ "$?" != "0" ]; then 
            echo "warning: cannot create TOP_INTEL_DIR $TOP_INTEL_DIR"
            exit 1
        fi
    fi

    local str=''

    for str in $LINK_STRUCT; do
        local DIR_VAR=$(_get_dir_var "$str"; echo $REPLY)
        local DIR_PATH=$(_get_dir_path "$str"; echo $REPLY)
        if [ -z "$DIR_VAR" ] || [ -z "$DIR_PATH" ]; then
            echo "warning: internal error while parsing '$str' string"
        fi
        if _is_dir "$str"; then
            # real dirs creation
            ICP12_CREATE_DIR "$DIR_PATH"
            if [ $? -ne 0 ]; then
                echo "warning: cannot create $DIR_VAR=$DIR_PATH"
                exit 1
            else
                eval ${DIR_VAR}_EXISTS=1
            fi
        else
            # link creation
            local LINK_POINT=$(_get_link_point "$str"; echo $REPLY)
            [[ -z "$LINK_POINT" ]] && echo "warning: internal error while parsing '$str' string - missed LINK_POINT"
            if [ "${SCRIPT_MODE}" == "slave" ]; then
                local IS_REWRITABLE=$(_is_rewritable_in_slave_mode "$str"; echo $REPLY)
            else
                local IS_REWRITABLE="y"
            fi
            ICP12_CREATE_DIR_LINK ${LINK_POINT} ${DIR_PATH} ${IS_REWRITABLE}
        fi
    done

    # copy this script
    ICP12_COPY_FILE $FULL_PROGRAM_PATH $INTEL_BIN_DIR
    ICP12_COPY_FILE $FULL_PROGRAM_PATH $INTEL_COMPOSERXE_DIR/bin

    # check which version directories "lib", "include" points to and try to set up all links in bin by binaries from this version
    CPRO_VER_SET=$(readlink $INTEL_COMPOSERXE_2011_LIB_DIR | sed "s/^.*\(composer_xe_2013.[0-9]\.[0-9][0-9][0-9]\).*$/\1/g")
    if [ -n "$CPRO_VER_SET" ]; then
        PACKAGEID="$CPRO_VER_SET"
        PKG_INSTALL_DESTINATION="$TOP_INTEL_DIR/$PACKAGEID"
    fi
    
    # create links for binary and non-env script program files
    for THIS_FILE in $INTEL_PROGRAM_FILES_LIST
    do
        # creating links on files in INTEL_COMPOSERXE_2011_BIN_DIR dir
        pushd $INTEL_COMPOSERXE_2011_BIN_DIR &> /dev/null
        ICP12_CREATE_FILE_LINK ../../$PACKAGEID/bin/$ARCH/$THIS_FILE $INTEL_COMPOSERXE_2011_BIN_DIR/$THIS_FILE
        popd &>/dev/null
        
        # creating links on files in INTEL_BIN_DIR dir
        pushd $INTEL_BIN_DIR &>/dev/null
        ICP12_CREATE_FILE_LINK ../composerxe/bin/$THIS_FILE $INTEL_BIN_DIR/$THIS_FILE
        popd &> /dev/null
    done

    # actions below we make always regardless what is installed (compiler links should anyway be updated by current compiler)
    # copy environment scripts and make script links 
    for THIS_FILE in $INTEL_ENV_FILE_LIST
    do
        # if env file is compilervars.[c]sh
        if [ "x$(echo $THIS_FILE | grep compilervars)" != "x" ]; then
            filetarget=$(find $PKG_INSTALL_DESTINATION -name "$THIS_FILE" -type f)
            filedestination=$(echo ${THIS_FILE} | sed -e"s/_global//g")
            cp -p ${filetarget} "$INTEL_COMPOSERXE_2011_BIN_DIR/${filedestination}" &>/dev/null
            VERBOSE_PRINT "Copied $THIS_FILE to $INTEL_COMPOSERXE_2011_BIN_DIR/${filedestination}"

            pushd $INTEL_BIN_DIR &>/dev/null
            ICP12_CREATE_FILE_LINK ../composerxe/bin/${filedestination} $INTEL_BIN_DIR/${filedestination}
            popd &> /dev/null
        else
            # don't create ifortvars.[c]sh if fortran isn't installed
            if [ "x$(echo $THIS_FILE | grep fort)" != "x" ] && [ ! -L $INTEL_BIN_DIR/ifort ]; then
                VERBOSE_PRINT "We should not have $THIS_FILE link, because Fortran package isn't installed"
		VERBOSE_PRINT "...trying to remove $INTEL_BIN_DIR/$THIS_FILE"
                pushd $INTEL_BIN_DIR &>/dev/null
		rm $RM_OPTIONS $THIS_FILE &>/dev/null
                popd &>/dev/null
                continue
            fi
            if [ "x$(echo $THIS_FILE | grep icc)" != "x" ] && [ ! -L $INTEL_BIN_DIR/icc ]; then
		VERBOSE_PRINT "We should not have $THIS_FILE link, because C++ package isn't installed (probably it was removed)"
		VERBOSE_PRINT "...trying to remove $INTEL_BIN_DIR/$THIS_FILE"
                pushd $INTEL_BIN_DIR &>/dev/null
		rm $RM_OPTIONS $THIS_FILE &>/dev/null
                popd &>/dev/null
                continue
            fi
            local C_OR_BASH=$(echo $THIS_FILE | cut -d'.' -f2 2>/dev/null)
            # create link to compilervars.[c]sh
            pushd $INTEL_BIN_DIR &>/dev/null
            ICP12_CREATE_FILE_LINK "../composerxe/bin/compilervars.${C_OR_BASH}" $THIS_FILE
            popd &>/dev/null
        fi
    done
    
    # support of SC foldermap
    pushd $INTEL_COMPOSERXE_2011_DIR &>/dev/null
    ICP12_CREATE_FILE_LINK "../$PACKAGEID/foldermap.sc.xml" "foldermap.sc.xml"
    popd &>/dev/null
    # support of SC foldermap - end
}

# -----------------------------------------------------------------------------
# remove all links
ICP12_RMLINKS() {
    VERBOSE_PRINT "ICP12_RMLINKS()"

    # support of SC foldermap
    if [ -e "${INTEL_COMPOSERXE_2011_DIR}/foldermap.sc.xml" ] ; then
        rm $RM_OPTIONS "${INTEL_COMPOSERXE_2011_DIR}/foldermap.sc.xml" &>/dev/null
    fi
    # support of SC foldermap - end

    local str=''

    for str in $LINK_STRUCT; do
        local DIR_VAR=$(_get_dir_var "$str"; echo $REPLY)
        local DIR_PATH=$(_get_dir_path "$str"; echo $REPLY)
        if [ -z "$DIR_VAR" ] || [ -z "$DIR_PATH" ]; then
            echo "warning: internal error while parsing '$str' string"
        fi
        VERBOSE_PRINT "removing $DIR_VAR=$DIR_PATH"
        # comparing real state of affairs with the LINK_STRUCT
        # if it's real directory and LINK_STRUCT says that it should be directory, then we
        # remove it; the same for links
        if _is_dir "$str"; then
            if [ -d ${DIR_PATH} ]; then
                rm $RM_OPTIONS ${DIR_PATH} &>/dev/null
                [[ $? -ne 0 ]] && echo "warning: problems removing $DIR_VAR"
            else
                VERBOSE_PRINT "RMLINKS: warning: script says '$DIR_PATH' should be real directory, but it's not so... doing nothing"
            fi
        else
            if [ -L ${DIR_PATH} ]; then
                rm $RM_OPTIONS ${DIR_PATH} &>/dev/null
                [[ $? -ne 0 ]] && echo "warning: problems removing $DIR_VAR"
            else
                VERBOSE_PRINT "RMLINKS: warning: script says '$DIR_PATH' should be symbolic link, but it's not so... doing nothing"
            fi
        fi
    done
}

# -----------------------------------------------------------------------------
# trap user abort - placeholder only - not implemented
ICP12_TRAP() {
    VERBOSE_PRINT "ICP12_TRAP()"
}

# -----------------------------------------------------------------------------
# this is a simple wrapper over "echo" for disabling output in non-verbose mode
VERBOSE_PRINT() {
    if [ "$VERBOSE_MODE" = "1" ]; then
        echo "$@"
    fi
}

# -----------------------------------------------------------------------------
# this function finds latest version of product installed on the system and returns its ID
FIND_LATEST_PROD_INSTANCE () {
    VERBOSE_PRINT "FIND_LATEST_PROD_INSTANCE()"

    CPRO_INSTALLED_VERSIONS=$(ls -A $TOP_INTEL_DIR | grep 'composer_xe_' | sed 's/composer_xe_//g' | grep -ve '^2013$' | grep -ve '2011' | sort -gr)
    local CPRO_LATEST_PACKAGE_NUMBER=$(echo "$CPRO_INSTALLED_VERSIONS" | cut -d'.' -f3 | sort -gr | head -1)

    if [ -z "$CPRO_INSTALLED_VERSIONS" ]; then
        VERBOSE_PRINT "No any versions installed"
        return 1
    else
        # Since versions are sorted in descending type, we can take the first line for getting latest version
        LATEST_VER=$(echo "$CPRO_INSTALLED_VERSIONS" | grep -e "^2013\.[0-9]*\.${CPRO_LATEST_PACKAGE_NUMBER}$")
    fi

    VERBOSE_PRINT "LATEST_VER=$LATEST_VER"
    
    return 0
}

# -----------------------------------------------------------------------------
# this function checks if file or directory is exist in one of the installed versions of CPro
# arguments:
#   - file - file in PKG_INSTALL_DESTINATION for check
#   - test_attr - attribute for bash 'test' function (can be '-d' or '-f' here)
CHECK_LATEST_VER_WHERE_FILE_PRESENT () {
    VERBOSE_PRINT "CHECK_LATEST_VER_WHERE_FILE_PRESENT()"

    local file="$1"
    local test_attr="$2"
    
    REPLY_VERSION=''

    IFS="$OLD_IFS"
    for VER in $CPRO_INSTALLED_VERSIONS; do
        VERBOSE_PRINT "trying to check presense of $TOP_INTEL_DIR/composer_xe_$VER/$file"
        if [ "$test_attr" "$TOP_INTEL_DIR/composer_xe_$VER/$file" ]; then
            VERBOSE_PRINT "...found!"
            REPLY_VERSION=$VER
            IFS=';'
            return 0
        fi
        VERBOSE_PRINT "...not present"
    done
    IFS=';'
    return 1
}

# -----------------------------------------------------------------------------
# initialize values for operation of this program 
# - invoked with arguments to parent script
ICP12_INIT() {
    VERBOSE_PRINT "ICP12_INIT()"

    # name of this script
    PROGRAM_NAME=$(basename $0)
    FULL_PROGRAM_PATH=$0

    # RM OPTIONS - options to the 'rm' program when removing pre-existing files
    RM_OPTIONS="-rf"

    if [ -z "$PKG_INSTALL_DESTINATION" ]; then
        PKG_INSTALL_DESTINATION="$TOP_INTEL_DIR/$PACKAGEID"
        VERBOSE_PRINT "PKG_INSTALL_DESTINATION=$PKG_INSTALL_DESTINATION"
    else
       # # We don't create symbolic links if PKG_INSTALL_DESTINATION is not in /opt/intel
       # if [ "x$(echo $PKG_INSTALL_DESTINATION | grep '/opt/intel')" = "x" ]; then
       #     echo "Error: product is installed not in /opt/intel. The symbolic links will not be created."
       #     exit 1
       # fi

        # reset the package ID to the one provided to script
        PACKAGEID=$(basename $PKG_INSTALL_DESTINATION) 
    fi
    VERBOSE_PRINT "PACKAGEID=$PACKAGEID"

    # Such structure is intented to simplify adding new links to the script
    # You can add new links/dirs as follows:
    # {d,l}     DIR_VAR    DIR_PATH   LINK_POINT
    # where
    #       {d,l} means whether this item should be real directory or symbolic link
    #       {y,n} - should this entry be rewritable in slave mode? - yes/no
    #       DIR_VAR - variable name containing resolved DIR_PATH (for further usage)
    #       DIR_PATH - directory/link path in link structure
    #       LINK_POINT - path which link should point to 
    #
    # NOTE: delimiters (;) between each string are mandatory!
    #       delimeters beewteen items within each string are just spaces
    # NOTE2: if you use some variable in DIR_PATH field, you should screen this variable by "\"
    #        (see structure below for example of usage)
    # NOTE3: Order of strings is important, because it's order of directories creation and 
    #        each next string is often based on previous one

    LINK_STRUCT="\
d n  INTEL_BIN_DIR                              \$TOP_INTEL_DIR/bin;\
d n  INTEL_COMPOSERXE_2011_DIR                  \$TOP_INTEL_DIR/composer_xe_2013;\
d n  INTEL_COMPOSERXE_2011_BIN_DIR              \$INTEL_COMPOSERXE_2011_DIR/bin;\
l n  INTEL_COMPOSERXE_DIR                       \$TOP_INTEL_DIR/composerxe                       composer_xe_2013;\
l n  INTEL_COMPOSERXE_BIN_SC                    \$INTEL_COMPOSERXE_2011_BIN_DIR/sourcechecker    ../../$PACKAGEID/bin/sourcechecker;\
l n  INTEL_COMPOSERXE_2011_PKG_BIN_DIR          \$INTEL_COMPOSERXE_2011_DIR/pkg_bin            ../$PACKAGEID/bin;\
l n  INTEL_COMPOSERXE_2011_LIB_DIR              \$INTEL_COMPOSERXE_2011_DIR/lib                ../$PACKAGEID/compiler/lib;\
l n  INTEL_COMPOSERXE_2011_INCLUDE_DIR          \$INTEL_COMPOSERXE_2011_DIR/include            ../$PACKAGEID/compiler/include;\
l n  INTEL_COMPOSERXE_2011_MAN_DIR              \$INTEL_COMPOSERXE_2011_DIR/man                ../$PACKAGEID/man;\
l y  INTEL_COMPOSERXE_2011_TBB_DIR              \$INTEL_COMPOSERXE_2011_DIR/tbb                ../$PACKAGEID/tbb;\
l y  INTEL_COMPOSERXE_2011_MKL_DIR              \$INTEL_COMPOSERXE_2011_DIR/mkl                ../$PACKAGEID/mkl;\
l y  INTEL_COMPOSERXE_2011_IPP_DIR              \$INTEL_COMPOSERXE_2011_DIR/ipp                ../$PACKAGEID/ipp;\
l y  INTEL_COMPOSERXE_2011_DEBUGGER_DIR         \$INTEL_COMPOSERXE_2011_DIR/debugger           ../$PACKAGEID/debugger;\
l n  INTEL_COMPOSERXE_2011_ECLIPSE_SUPPORT_DIR  \$INTEL_COMPOSERXE_2011_DIR/eclipse_support    ../$PACKAGEID/eclipse_support;\
l n  INTEL_COMPOSERXE_2011_DOCUMENTATION_DIR    \$INTEL_COMPOSERXE_2011_DIR/Documentation      ../$PACKAGEID/Documentation;\
l n  INTEL_COMPOSERXE_2011_SAMPLES_DIR          \$INTEL_COMPOSERXE_2011_DIR/Samples            ../$PACKAGEID/Samples;\
l n  INTEL_LIB_DIR                               \$TOP_INTEL_DIR/lib                             composerxe/lib;\
l n  INTEL_INCLUDE_DIR                           \$TOP_INTEL_DIR/include                         composerxe/include;\
l n  INTEL_MAN_DIR                               \$TOP_INTEL_DIR/man                             composerxe/man;\
l y  INTEL_IPP_DIR                               \$TOP_INTEL_DIR/ipp                             composerxe/ipp;\
l y  INTEL_MKL_DIR                               \$TOP_INTEL_DIR/mkl                             composerxe/mkl;\
l y  INTEL_TBB_DIR                               \$TOP_INTEL_DIR/tbb                             composerxe/tbb"

    _fill_link_struct_vars

    # Intel environment files list
    INTEL_ENV_FILE_LIST="\
compilervars_global.sh;\
compilervars_global.csh;\
iccvars.sh;\
iccvars.csh;\
ifortvars.sh;\
ifortvars.csh"

    # Intel program files list
    INTEL_PROGRAM_FILES_LIST="\
codecov;\
fpp;\
icc;\
icpc;\
ifort;\
map_opts;\
profdcg;\
profmerge;\
proforder;\
tselect;\
xiar;\
xild;\
idb;\
idbc;\
inspxe-inject;\
inspxe-runsc;\
inspxe-wrap"
}

# -----------------------------------------------------------------------------
# this function fills variables from the 3rd column of LINK_STRUCT by values from the 4th
_fill_link_struct_vars() {
    local str=''

    for str in $LINK_STRUCT; do
        str=$(echo $str | tr -s [:blank:] | sed 's/^ *//g')
        local var_name=$(echo $str |  cut -d' ' -f3)
        local var_value=$(eval echo $str | cut -d' ' -f4)
        eval $var_name="$var_value"
    done
}
# -----------------------------------------------------------------------------
# Simple functions for working with LINK_STRUCT
# - argument - string from LINK_STRUCT
_is_rewritable_in_slave_mode() {
    local str=$(echo $1 | tr -s [:blank:] | sed 's/^ *//g')
    REPLY=$(echo $str | cut -d' ' -f2)
    if [ -z $REPLY ]; then
        return 1
    else
        return 0
    fi
}

# -----------------------------------------------------------------------------
# - argument - string from LINK_STRUCT
_get_dir_var() {
    local str=$(echo $1 | tr -s [:blank:] | sed 's/^ *//g')
    REPLY=$(echo $str | cut -d' ' -f3)
    if [ -z $REPLY ]; then
        return 1
    else
        return 0
    fi
}

# -----------------------------------------------------------------------------
# - argument - string from LINK_STRUCT
_get_dir_path() {
    local str=$(echo $1 | tr -s [:blank:] | sed 's/^ *//g')
    REPLY=$(eval echo $str | cut -d' ' -f4)
    if [ -z $REPLY ]; then
        return 1
    else
        return 0
    fi
}

# -----------------------------------------------------------------------------
# - argument - string from LINK_STRUCT
_get_link_point() {
    local str=$(echo $1 | tr -s [:blank:] | sed 's/^ *//g')
    REPLY=$(echo $str | cut -d' ' -f5)
    if [ -z $REPLY ]; then
        return 1
    else
        return 0
    fi
}

# -----------------------------------------------------------------------------
# - argument - string from LINK_STRUCT
_is_dir() {
    local str=$(echo $1 | tr -s [:blank:] | sed 's/^ *//g')
    if [ "x$(echo $str | cut -d' ' -f1)" = "xd" ]; then
        return 0
    else
        return 1
    fi
}

# -----------------------------------------------------------------------------
# this function returns ARCH for current package
SET_ARCH() {
    VERBOSE_PRINT "SET_ARCH()"
    # my architecture (either "ia32" or "intel64")
    UNAME=$(uname -m)
    if [ "$UNAME" = "i686" ]; then
        ARCH=ia32
    else
        # this is to adress case if 32-bit only package is installed on intel64 system (the ARCH should be ia32 in such case)
        if [ -d "$PKG_INSTALL_DESTINATION/bin/intel64" ]; then
            ARCH=intel64
        else
            ARCH=ia32
        fi
    fi
    VERBOSE_PRINT "ARCH=$ARCH"
}

# -----------------------------------------------------------------------------
# parse command line
# note that arguments -i|-e|-s|-u are alternatives and cannot be choosen simultaneously

PARSE_CMD_ARGS() {
    iflg=0
    eflg=0
    sflg=0
    uflg=0
    parse_cmd_err=0
    
    while getopts ":iesup:l:vm:" opt; do
        case $opt in
            i)
                if [ $eflg -eq 0 ] && [ $sflg -eq 0 ] && [ $uflg -eq 0 ]; then
                    TO_DO="install"
                    iflg=1
                else
                    parse_cmd_err=1
                fi
                ;;
            e)
                if [ $iflg -eq 0 ] && [ $sflg -eq 0 ] && [ $uflg -eq 0 ]; then
                    TO_DO="erase"
                    eflg=1
                else
                    parse_cmd_err=1
                fi
                ;;
            s)
                if [ $iflg -eq 0 ] && [ $eflg -eq 0 ] && [ $uflg -eq 0 ]; then
                    TO_DO="status"
                    sflg=1
                else
                    parse_cmd_err=1
                fi
                ;;
            u)
                if [ $iflg -eq 0 ] && [ $eflg -eq 0 ] && [ $sflg -eq 0 ]; then
                    TO_DO="update"
                    uflg=1
                else
                    parse_cmd_err=1
                fi
                ;;
            p)
                PKG_INSTALL_DESTINATION="$OPTARG"
                if [ ! -d "$PKG_INSTALL_DESTINATION" ]; then
                    echo "Installation directory $PKG_INSTALL_DESTINATION does not exist"
                fi
                ;;
            l)
                VERBOSE_PRINT "HIDDEN OPTION: user changed TOP_INTEL_DIR to '$OPTARG'"
                TOP_INTEL_DIR="$OPTARG"
                if [ ! -d "$TOP_INTEL_DIR" ]; then
                    echo "Intel product directory $TOP_INTEL_DIR does not exist"
                fi
                ;;
            v)
                VERBOSE_MODE=1
                ;;
            m)
                VERBOSE_PRINT "SCRIPT MODE: $OPTARG"
                SCRIPT_MODE="$OPTARG"
                if [ "x$SCRIPT_MODE" != "xslave" ] && [ "x$SCRIPT_MODE" != "xmaster" ]; then
                    parse_cmd_err=2
                fi
                ;;
                
            \?)
                echo "Invalid option: -$OPTARG" >&2
                exit 1
                ;;
            :)
                echo "ERROR: Option -$OPTARG requires an argument." >&2
                exit 1
                ;;
        esac
    done
   
    case $parse_cmd_err in
        1)
            echo "error in parsing command line arguments: parameters -i -u -e or -s are alternatives"
            exit 1
            ;;
        2)
            echo "error in parsing command line arguments: please indicate right script mode: -m slave or -m master"
            exit 1
            ;;
    esac

}

# -----------------------------------------------------------------------------
# show usage
ICP12_USAGE() {
    VERBOSE_PRINT "ICP12_TRAP()"
    
    echo "Script $0 - USAGE"
    echo
    echo "$0 -i|-e|-s|-u [ -p <PATH_TO_PACKAGE> ] [ -l <PATH_TO_LINKS> ] [ -m <MODE> ] [ -v ]"
    echo
    echo "  -i | -e | -s | -u   - 'install links and script / erase links and script / return links status / update links on the latest package' respectively"
    echo "         uses /opt/intel for links target by default, please retarget it using -l key to your specific location for non-root users"
    echo "  -p PATH_TO_PACKAGE  - full path to the installed package of Composer XE 2013"
    echo "         (e.g., /opt/intel/composer_xe_2013.1.117)"
    echo "  -m <MODE>           - mode which script operates in ( master (default) or slave )"
    echo "  -v                  - enables VERBOSE mode"
    echo
    echo "Examples:"
    echo 
    echo "$0 -s"
    echo "     reports current status of installation and links"
    echo
    echo "$0 -i -p /opt/intel/composer_xe_2013.1.117"
    echo "     will create links to package installed into /opt/intel/composer_xe_2013.1.117 directory)"
    echo
    echo "$0 -i -l /home/user"
    echo "     will operate with /home/user location and manage links to packages installed into /home/user (useful for non-root users)"
    echo
    echo "$0 -e"
    echo "     will uninstall existed links and remove this script from symbolic links location"
    echo
    echo "Copyright 2006-2012, Intel Corporation, All Rights Reserved."
}

# -------- Script starts here --------

OLD_IFS=$IFS
IFS=";"

# locally hardcoded for test purposes
#DEFAULT_PACKAGEID="composerxe-2011.0.023"
DEFAULT_PACKAGEID="composer_xe_2013.1.117"

PACKAGEID=$DEFAULT_PACKAGEID

PACKAGENUM=$(echo $PACKAGEID | sed -e"s/composer_xe_2013\.[0-9]\.//g")
    
# parse script's command line
PARSE_CMD_ARGS $@

if [ -z "${TO_DO}" ]; then
    echo "error: missing mandatory arguments (-i, -u, -e or -s)"
    echo
    ICP12_USAGE
    exit 1
fi

if [ -z "${SCRIPT_MODE}" ]; then
    VERBOSE_PRINT "WARNING: No script mode defined... Defaulted to 'master'."
    SCRIPT_MODE="master"
fi

# top Intel product directory name (it shouldn't be changed in theory)
if [ -z "$TOP_INTEL_DIR" ]; then
    # if user is root
    if [ $(id -u) -eq 0 ]; then
        TOP_INTEL_DIR="/opt/intel"
    else
        # " -- if users do not install the product as root, they do not get the symbolic links (see PRD CR)
        echo "Error: user is not root. The symbolic links will not be created."
        exit 1
    fi
fi
VERBOSE_PRINT "TOP_INTEL_DIR=$TOP_INTEL_DIR"

ICP12_INIT

case "${TO_DO}" in
    install)
        ICP12_DETECT
        # report file / link status before making any links
        [[ "$VERBOSE_MODE" = "1" ]] && ICP12_REPORT
        ICP12_MKLINKS
        ;;
    erase)
        ICP12_RMLINKS
        ;;
    status)
        ICP12_DETECT
        ICP12_REPORT
        exit 0
        ;;
    update)
        VERBOSE_PRINT "UPDATE MODE"
        FIND_LATEST_PROD_INSTANCE
        if [ $? -eq 0 ]; then
            # we ignore PKG_INSTALL_DESTINATION when we do update
            # (because we use latest version of Compiler, but not exactly specified one)
            PKG_INSTALL_DESTINATION=''
            PACKAGEID="composer_xe_$LATEST_VER"
            # reinit
            ICP12_INIT
            ICP12_DETECT
            ICP12_MKLINKS
        else
            ICP12_RMLINKS
        fi
        ;;
    *)
        echo "Internal error: no script arguments defined"
        exit 1
        ;;
esac
