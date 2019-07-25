#!/bin/bash
# Given a command string, neatly divide all arguments between lines and print
# the full command.

argString=$1
argList=()
buffer=''
for word in $argString; do
    if [ -z "$buffer" ] && [ ${word:0:1} == '"' ] && [ ${word: -1} != '"' ]; then
        buffer=$word
    elif [ "${word: -1}" == '"' ]; then
        argList+=("$buffer $word")
        buffer=''
    else
        argList+=($word)
    fi
done
if [ -n "$buffer" ]; then
    argList+=($buffer)
fi

maxLine=77
buffer=''
for arg in "${argList[@]}"; do
    arg="$(echo $arg | xargs)"
    bufLen=`expr length "$buffer"`
    argLen=`expr length "$arg"`
    if (( ($bufLen + $argLen) > $maxLine )); then
        if (( $bufLen > 0 )); then
            echo "    $buffer"'\'
            buffer=''
        fi
    fi
    buffer="$buffer$arg "
done
if [ -n "$buffer" ]; then
    echo "    $buffer"'\'
fi


