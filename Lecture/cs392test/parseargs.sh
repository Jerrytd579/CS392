#!/bin/bash

permissions_flag=0
size_flag=0

while getopts ":ps" option; do
    case "$option" in
        p) permissions_flag=1
           ;;
        s) size_flag=1
           ;;
        ?) echo "Error: Unknown option '-$OPTARG'." >&2
           exit 1
           ;;
    esac
done

shift "$((OPTIND-1))"
count=$(( permissions_flag + size_flag ))
for f in $@; do
    file_listing=$(ls -l "$f" 2>/dev/null)
    if [ ! -z "$file_listing" ]; then
        if [ $count -eq 0 ]; then
            echo "$f"
        else
            echo -n "$f"
        fi
        if [ $permissions_flag -eq 1 ]; then
            permissions=$(cut -d' ' -f1 <<<$file_listing)
            if [ $count -eq 1 ]; then
                echo ": $permissions"
            else
                echo -n ": $permissions"
            fi
        fi
        if [ $size_flag -eq 1 ]; then
            file_size=$(cut -d' ' -f5 <<<$file_listing)
            echo ": $file_size bytes"
        fi
    fi
done

exit 0
