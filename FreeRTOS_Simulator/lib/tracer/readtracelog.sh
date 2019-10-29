#!/bin/sh
if test ! -f "$1"; then
    echo "Error: executable $1 does not exist."
    exit 1
fi
if test ! -f "$2"; then
    echo "Error: trace log $2 does not exist."
    exit 1
fi
EXECUTABLE="$1"
TRACELOG="$2"
LINE="$(sed -n '2p' $TRACELOG)"
MAIN_LINE=($LINE)
MAIN_ADDR=${MAIN_LINE[1]}
LINE="$(nm -an $EXECUTABLE | grep "T main")"
MAIN_LINE=($LINE)
MAIN_OFFSET=${MAIN_LINE[0]}
OFFSET=$((MAIN_ADDR - "0x"$MAIN_OFFSET))
OFFSET=$(printf "0x%x" $OFFSET)
while read LINETYPE FADDR CADDR CTIME; do
    FADDR_O=$(printf "0x%x" $((FADDR - OFFSET)))
    CADDR_O=$(printf "0x%x" $((CADDR - OFFSET)))
    FNAME="$(addr2line -f -e ${EXECUTABLE} ${FADDR_O} | head -1)"
    CDATE="$(date -Iseconds -d @${CTIME})"
    if test "${LINETYPE}" = "e"; then
        CNAME="$(addr2line -f -e ${EXECUTABLE} ${CADDR_O} | head -1)"
        CLINE="$(addr2line -s -e ${EXECUTABLE} ${CADDR_O})"
        echo "Enter ${FNAME} at ${CDATE}, called from ${CNAME} (${CLINE})"
    fi
    if test "${LINETYPE}" = "x"; then
        echo "Exit  ${FNAME} at ${CDATE}"
    fi
done <"${TRACELOG}"
