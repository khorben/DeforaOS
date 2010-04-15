#/bin/sh
#$Id$
#Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>
#This file is part of DeforaOS Network Probe
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, version 3 of the License.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.



#functions
#create
create()
{
	mkdir -p "$i"
	rrdtool create "$i/uptime.rrd" --start "`date +%s`" \
		--step 10 \
		DS:uptime:GAUGE:120:0:U \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/load.rrd" --start "`date +%s`" \
		--step 10 \
		DS:load1:GAUGE:30:0:U \
		DS:load5:GAUGE:30:0:U \
		DS:load15:GAUGE:30:0:U \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/ram.rrd" --start "`date +%s`" \
		--step 10 \
		DS:ramtotal:GAUGE:30:0:U \
		DS:ramfree:GAUGE:30:0:U \
		DS:ramshared:GAUGE:30:0:U \
		DS:rambuffer:GAUGE:30:0:U \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/swap.rrd" --start "`date +%s`" \
		--step 10 \
		DS:swaptotal:GAUGE:30:0:U \
		DS:swapfree:GAUGE:30:0:U \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/users.rrd" --start "`date +%s`" \
		--step 10 \
		DS:users:GAUGE:30:0:65536 \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/procs.rrd" --start "`date +%s`" \
		--step 10 \
		DS:procs:GAUGE:30:0:65536 \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360

	rrdtool create "$i/eth0.rrd" --start "`date +%s`" \
		--step 10 \
		DS:ifrxbytes:COUNTER:30:0:U \
		DS:iftxbytes:COUNTER:30:0:U \
		RRA:AVERAGE:0.5:1:360 \
		RRA:AVERAGE:0.5:24:360 \
		RRA:AVERAGE:0.5:168:360
}


#usage
usage()
{
	echo "Usage: create.sh host..." 1>&2
	return 1;
}


#main
if [ $# -lt 1 ]; then
	usage
	exit $?
fi
for i in $@; do
	create "$i"
done
