#!/bin/sh
#$Id$



#usage
_usage()
{
	echo "Usage: ./appbroker.sh target" 1>&2
	return 1
}


#main
args=`getopt P: $*`
if [ $? -ne 0 ]; then
	_usage
	exit $?
fi
set -- $args
while [ $# -gt 0 ]; do
	case "$1" in
		-P)
			#we can ignore it
			shift
			;;
		--)
			shift
			break
			;;
	esac
	shift
done

if [ $# -ne 1 ]; then
	_usage
	exit $?
fi
APPINTERFACE="${1%%.h}.interface"
AppBroker -o "$1" "$APPINTERFACE"
