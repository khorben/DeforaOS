#!/bin/sh
#$Id$



#usage
usage()
{
	echo "Usage: ./appbroker.sh target" 1>&2
	return 1
}


#main
if [ $# -ne 1 ]; then
	usage
	exit $?
fi
APPINTERFACE="${1%%.h}.interface"
AppBroker -o "$1" "$APPINTERFACE"
