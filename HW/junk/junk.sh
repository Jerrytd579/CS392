###############################################################################
# Author: Jerry Cheng, Andrew Chuah
# Date: 5/19/2020
# Pledge: I pledge my honor that I have abided by the Stevens Honor System.
# Description: junk.sh
###############################################################################
#!/bin/bash

readonly DIRNAME=~/.junk
mkdir -p $DIRNAME

helpflag=0
listjunkflag=0
purgelistflag=0
argsflag=0

usage_message(){
cat << USAGE
Usage: `basename $0` [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
USAGE
}

while getopts ":hlp" option; do
    case "$option" in
        h) helpflag=1
            ;;
        l) listjunkflag=1
            ;;
        p) purgelistflag=1
            ;;
        ?) echo "Error: Unknown option '-$OPTARG'." >&2
            usage_message
            exit 1
            ;;
    esac
done

# if more than one flag is supplied, throw error
if [ $(( helpflag + listjunkflag + purgelistflag )) -gt 1 ]; then
    echo "Error: Too many options enabled."
    usage_message
    exit 1
fi

shift "$((OPTIND-1))"

# checks if file doesn't exist, else moves file into ~/.junk
process_files(){
    if [ ! -e $1 ]; then
        echo "Warning: '$1' not found."
        exit 1
    else    
        mv -f "$1" $DIRNAME
    fi
}

for f in $@; do
    argsflag=$((argsflag+1))
    # if one or more flags are specified and files are supplied, throw error
    if [[ $(( helpflag + listjunkflag + purgelistflag )) -gt 0 && argsflag -gt 0 ]]; then
        echo "Error: Too many options enabled."
        usage_message
        exit 1
    fi

    process_files "$f"
done

# three flags functionality
if [ $helpflag  -eq 1 ]; then
    usage_message
    exit 0
fi

if [ $listjunkflag -eq 1 ]; then
    ls -lAF $DIRNAME
    exit 0
fi

if [ $purgelistflag -eq 1 ]; then
    #rm -rfv ~/.junk/*
    find $DIRNAME -mindepth 1 -delete
    exit 0
fi


exit 0