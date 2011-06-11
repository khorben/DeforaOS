<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));


//lang
$text['ALL_HOSTS'] = 'All hosts';
$text['COMMENT'] = 'Comment';
$text['CONFIGURATION_ERROR'] = 'Configuration error';
$text['CREATE'] = 'Create';
$text['GRAPH_LIST'] = 'Graph list';
$text['HOST_LIST'] = 'Host list';
$text['HOSTNAME'] = 'Hostname';
$text['LOAD_AVERAGE'] = 'Load average';
$text['LOGGED_USERS'] = 'Logged users';
$text['MONITORING'] = 'Monitoring';
$text['MONITORING_ADMINISTRATION'] = 'Monitoring administration';
$text['MEMORY_USAGE'] = 'Memory usage';
$text['NETWORK_TRAFFIC'] = 'Network traffic';
$text['NEW_HOST'] = 'New host';
$text['PROCESS_COUNT'] = 'Process count';
$text['SETTINGS'] = 'Settings';
$text['SWAP_USAGE'] = 'Swap usage';
$text['UPTIME'] = 'Uptime';
$text['VOLUME_USAGE'] = 'Volume usage';

?>
