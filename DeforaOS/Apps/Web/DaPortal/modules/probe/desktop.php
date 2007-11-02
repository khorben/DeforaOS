<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));


$title = 'Monitoring';
$icon = 'host.png';
$admin = 1;
$list = 1;
$actions = array('graph_list' => array('title' => 'Graphs'),
		'host_list' => array('title' => 'Hosts',
			'args' => 'action=host_list'));
global $lang;
if($lang == 'fr')
{
	$actions['graph_list']['title'] = 'Graphes';
	$actions['host_list']['title'] = 'Machines';
}
$text['UPTIME'] = 'Uptime';
$text['LOAD_AVERAGE'] = 'Load average';
$text['LOGGED_USERS'] = 'Logged users';
$text['MEMORY_USAGE'] = 'Memory usage';
$text['NETWORK_TRAFFIC'] = 'Network traffic';
$text['PROCESS_COUNT'] = 'Process count';
$text['SWAP_USAGE'] = 'Swap usage';
$text['VOLUME_USAGE'] = 'Volume usage';
if($lang == 'fr')
{
	$text['LOAD_AVERAGE'] = 'Charge moyenne';
	$text['LOGGED_USERS'] = 'Utilisateurs connectés';
	$text['MEMORY_USAGE'] = 'Utilisation mémoire';
	$text['NETWORK_TRAFFIC'] = 'Activité réseau';
	$text['PROCESS_COUNT'] = 'Nombre de processus';
	$text['SWAP_USAGE'] = 'Utilisation échange';
	$text['VOLUME_USAGE'] = 'Utilisation des volumes';
}
_lang($text);

$actions['graph_list']['actions'] = array();
$actions['graph_list']['actions'][] = array('title' => UPTIME,
		'args' => 'type=uptime');
$actions['graph_list']['actions'][] = array('title' => LOAD_AVERAGE,
		'args' => 'type=load');
$actions['graph_list']['actions'][] = array('title' => MEMORY_USAGE,
		'args' => 'type=ram');
$actions['graph_list']['actions'][] = array('title' => SWAP_USAGE,
		'args' => 'type=swap');
$actions['graph_list']['actions'][] = array('title' => LOGGED_USERS,
		'args' => 'type=users');
$actions['graph_list']['actions'][] = array('title' => PROCESS_COUNT,
		'args' => 'type=procs');
$actions['graph_list']['actions'][] = array('title' => NETWORK_TRAFFIC,
		'args' => 'type=iface');
$actions['graph_list']['actions'][] = array('title' => VOLUME_USAGE,
		'args' => 'type=vol');

$hosts = _sql_array('SELECT title, content_id AS id'
		.' FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND daportal_module.name='probe'"
		." AND daportal_content.enabled='1' ORDER BY title ASC");
if(is_array($hosts) && count($hosts))
{
	$actions['host_list']['actions'] = array();
	foreach($hosts as $h)
		$actions['host_list']['actions'][] = array(
				'title' => $h['title'],
				'args' => 'id='.$h['id']);
}

?>
