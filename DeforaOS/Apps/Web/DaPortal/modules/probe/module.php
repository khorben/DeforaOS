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
//TODO:
//- fix configuration when name has a space
//- use title as an actual title and store the hostname in a dedicated table?



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
include('./modules/probe/lang.php');
global $lang;
if($lang == 'fr')
{
	include('./modules/probe/lang.fr.php');
}
_lang($text);


//private
//constants
global $probe_types;
$probe_types = array();

$probe_types['uptime'] = array('name' => 'uptime',
		'unit' => 'hours', 'def' => array('uptime'),
		'cdef' => array('ruptime' => 'uptime,3600,/'),
		'data' => ' AREA:ruptime#ffc0c0'
		.' LINE1:ruptime#ef0f0f:"Uptime\t\g"'
		.' GPRINT:ruptime:LAST:"Current\: %.0lf h\t\g"'
		.' GPRINT:ruptime:AVERAGE:"Average\: %.0lf h\t\g"'
		.' GPRINT:ruptime:MAX:"Maximum\: %.0lf h"');

$probe_types['load'] = array('name' => 'load average', 'args' => '-l 0',
		'def' => array('load1', 'load5', 'load15'),
		'cdef' => array('rload1' => 'load1,65536,/',
			'rload5' => 'load15,65536,/',
			'rload15' => 'load15,65536,/'),
		'data' => ' AREA:rload1#ffef5f AREA:rload5#ffbf5f'
		.' AREA:rload15#ff8f5f LINE1:rload1#ffdf00:"Load 1 min\t\g"'
		.' GPRINT:rload1:LAST:"Current\: %.2lf\t\g"'
		.' GPRINT:rload1:AVERAGE:"Average\: %.2lf\t\g"'
		.' GPRINT:rload1:MAX:"Maximum\: %.2lf\n"'
		.' LINE1:rload5#ffaf00:"Load 5 min\t\g"'
		.' GPRINT:rload5:LAST:"Current\: %.2lf\t\g"'
		.' GPRINT:rload5:AVERAGE:"Average\: %.2lf\t\g"'
		.' GPRINT:rload5:MAX:"Maximum\: %.2lf\n"'
		.' LINE1:rload15#ff7f00:"Load 15 min  "'
		.' GPRINT:rload15:LAST:"Current\: %.2lf\t\g"'
		.' GPRINT:rload15:AVERAGE:"Average\: %.2lf\t\g"'
		.' GPRINT:rload15:MAX:"Maximum\: %.2lf"');

$probe_types['ram'] = array('name' => 'memory usage', 'unit' => 'bytes',
		'base' => '1024', 'args' => '-l 0',
		'def' => array('ramtotal', 'ramfree', 'ramshared', 'rambuffer'),
		'cdef' => array('pramtotal' => 'ramtotal,1024,/,1024,/',
			'pramfree' => 'ramfree,1024,/,1024,/',
			'pramshared' => 'ramshared,1024,/,1024,/',
			'prambuffer' => 'rambuffer,1024,/,1024,/'),
		'data' => ' AREA:ramtotal#ff0000:"Total   "'
		.' GPRINT:pramtotal:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:pramtotal:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:pramtotal:MAX:"Maximum\: %.0lf MB\n"'
		.' AREA:ramfree#0000ff:"Free    "'
		.' GPRINT:pramfree:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:pramfree:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:pramfree:MAX:"Maximum\: %.0lf MB\n"'
		.' STACK:ramshared#00ffff:"Shared  "'
		.' GPRINT:pramshared:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:pramshared:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:pramshared:MAX:"Maximum\: %.0lf MB\n"'
		.' STACK:rambuffer#00ff00:"Buffer  "'
		.' GPRINT:prambuffer:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:prambuffer:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:prambuffer:LAST:"Maximum\: %.0lf MB\g"');

$probe_types['swap'] = array('name' => 'swap usage', 'unit' => 'bytes',
		'base' => '1024', 'args' => '-l 0',
		'def' => array('swaptotal', 'swapfree'),
		'cdef' => array('pswaptotal' => 'swaptotal,1024,/,1024,/',
				'pswapfree' => 'swapfree,1024,/,1024,/'),
		'data' => ' AREA:swaptotal#ff0000:"Total\t\g"'
		.' GPRINT:pswaptotal:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:pswaptotal:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:pswaptotal:MAX:"Maximum\: %.0lf MB\n"'
		.' AREA:swapfree#0000ff:"Free\t\g"'
		.' GPRINT:pswapfree:LAST:"Current\: %.0lf MB\t\g"'
		.' GPRINT:pswapfree:AVERAGE:"Average\: %.0lf MB\t\g"'
		.' GPRINT:pswapfree:MAX:"Maximum\: %.0lf MB\g"');

$probe_types['users'] = array('name' => 'logged users',
		'thumbnail' => 'icons/48x48/users.png', 'unit' => 'users',
		'def' => array('users'), 'data' => ' AREA:users#b0b0ff'
		.' LINE1:users#0f0fef:"Logged users\t\g"'
		.' GPRINT:users:LAST:"Current\: %.0lf\t\g"'
		.' GPRINT:users:AVERAGE:"Average\: %.0lf\t\g"'
		.' GPRINT:users:MAX:"Maximum\: %.0lf"');

$probe_types['procs'] = array('name' => 'process count', 'unit' => 'process',
		'def' => array('procs'), 'data' => ' AREA:procs#b0b0ff'
		.' LINE1:procs#0f0fef:"Process count\t\g"'
		.' GPRINT:procs:LAST:"Current\: %.0lf \t\g"'
		.' GPRINT:procs:AVERAGE:"Average\: %.0lf \t\g"'
		.' GPRINT:procs:MAX:"Maximum\: %.0lf"');

$probe_types['iface'] = array('name' => 'network traffic', 'unit' => 'Bps',
		'def' => array('ifrxbytes', 'iftxbytes'),
		'cdef' => array('ifrxkb' => 'ifrxbytes,1024,/',
			'iftxkb' => 'iftxbytes,1024,/'),
		'data' => ' AREA:ifrxbytes#b0ffb0'
		.' LINE1:ifrxbytes#0fef0f:"RX\t\g"'
		.' GPRINT:ifrxkb:LAST:"Current\: %.0lf KBps\t\g"'
		.' GPRINT:ifrxkb:AVERAGE:"Average\: %.0lf KBps\t\g"'
		.' GPRINT:ifrxkb:MAX:"Maximum\: %.0lf KBps\n"'
		.' LINE1:iftxbytes#0f0fef:"TX\t\g"'
		.' GPRINT:iftxkb:LAST:"Current\: %.0lf KBps\t\g"'
		.' GPRINT:iftxkb:AVERAGE:"Average\: %.0lf KBps\t\g"'
		.' GPRINT:iftxkb:MAX:"Maximum\: %.0lf KBps\g"',
		'params' => array('eth0', 'eth1', 'pppoe0', 'sip0', 'sip1',
			'sip2', 'sip3', 'sip4', 'sip5', 'sip6', 'ex0'));

$probe_types['vol'] = array('name' => 'volume usage', 'unit' => 'MB',
		'args' => '-l 0', 'def' => array('voltotal', 'volfree'),
		//FIXME block size may not be 4
		'cdef' => array('pvoltotal' => 'voltotal,1024,/',
			'pvolfree' => 'volfree,1024,/'),
		'data' => ' AREA:pvoltotal#ff0000:"Total\:\g"'
		.' GPRINT:pvoltotal:LAST:" %.0lf MB"'
		.' AREA:pvolfree#0000ff:"Free\:\g"'
		.' GPRINT:pvolfree:LAST:" %.0lf MB"',
		'params' => array('/home', '/usr', '/var'));


//functions
function _host_list()
{
	$module_id = _module_id('probe');
	return _sql_array('SELECT content_id AS id, title AS name, title'
			.' FROM daportal_content'
			." WHERE module_id='$module_id' AND enabled='1'"
			.' ORDER BY id ASC');
}

function _host_graph($id, $type, $time, $param = FALSE)
{
	global $probe_types;
	static $probe = FALSE;

	if($probe == FALSE && ($probe = _config_get('probe', 'RRD_repository'))
			== FALSE)
		return _error(CONFIGURATION_ERROR);
	if(($hostname = _host_title($id)) == FALSE
			|| !array_key_exists($type, $probe_types))
		return _error(INVALID_ARGUMENT);
	$ret = $probe_types[$type];
	$ret['id'] = $id;
	$ret['type'] = $type;
	$ret['title'] = ucfirst($ret['name']).': '.$hostname;
	switch($time)
	{
		case 'week':	$start = '-604800';	break;
		case 'day':	$start = '-86400';	break;
		case 'hour':
		default:	$start = '-3600';
				$time = 'hour';		break;
	}
	if($param != FALSE) //FIXME ugly
	{
		//$rrd = $probe.'/'.$hostname.'_'.$type.'/'.$param.'.rrd';
		$rrd = $probe.'/'.$hostname.'_'.$param.'.rrd';
		$sep = '_';
		$rrd2 = $probe.'/'.$hostname.'/'.$param.'.rrd';
		if(is_readable($rrd2))
		{
			$rrd = $rrd2;
			$sep = '/';
		}
		else if(!is_readable($rrd))
			return;
		$ret['title'].=' '.$param;
		$ret['img'] = 'tmp/'.$hostname.$sep.$param.'_'.$time.'.png';
	}
	else
	{
		$rrd = $probe.'/'.$hostname.'_'.$type.'.rrd';
		$ret['img'] = 'tmp/'.$hostname.'_'.$type.'_'.$time.'.png';
	}
	_info('rrd: '.$rrd);
	$file = $_SERVER['DOCUMENT_ROOT'].'/'.$ret['img'];
	//FIXME hardcoded path + not always true (if, vol)
	if(is_readable($file) && ($st = stat($file)) != FALSE
			&& $st['mtime'] + 30 > time())
		return $ret;
	$cmd = 'rrdtool graph --slope-mode '.escapeshellarg($file)
		.' --start '.$start.' --imgformat PNG'
		.' -c BACK#dcdad5 -c SHADEA#ffffff -c SHADEB#9e9a91';
	if(isset($ret['base']))
		$cmd.=' --base '.$ret['base'];
	foreach($ret['def'] as $d)
		$cmd.=" 'DEF:".$d.'='.$rrd.':'.$d.":AVERAGE'";
	if(isset($ret['cdef']))
	{
		$keys = array_keys($ret['cdef']);
		foreach($keys as $k)
			$cmd.=" 'CDEF:".$k.'='.$ret['cdef'][$k]."'";
	}
	if(isset($ret['data']))
		$cmd.=$ret['data'];
	if(isset($ret['args']))
		$cmd.=' '.$ret['args'];
	$cmd.=" --title '".$hostname;
	if($param != FALSE)
		$cmd.=' '.$param;
	$cmd.=' '.$ret['name'].' (last '.$time.")'";
	$cmd.=" --vertical-label '".(isset($ret['unit']) ? $ret['unit']
		: ' ')."'";
	_info('exec: '.$cmd);
	//FIXME check potential command insertion through hostname/title/etc
	//	currently vulnerable to single-quote injections
	exec($cmd);
	return $ret;
}

function _host_title($id)
{
	static $cache = array();

	if(array_key_exists($id, $cache))
		return $cache[$id];
	$module_id = _module_id('probe');
	$cache[$id] = _sql_single('SELECT title FROM daportal_content'
		." WHERE module_id='$module_id' AND content_id='$id'"
		." AND enabled='1'");
	return $cache[$id];
}

function _probe_toolbar()
{
	global $probe_types;

	$ret = array();
	$keys = array_keys($probe_types);
	foreach($keys as $k)
	{
		$t = &$probe_types[$k];
		$entry = array('name' => ucfirst($t['name']), 'type' => $k);
		$entry['title'] = $entry['name'];
		if(isset($t['icon']))
			$entry['icon'] = $t['icon'];
		$ret[] = $entry;
	}
	return $ret;
}


//public
function probe_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title probe">'._html_safe(MONITORING_ADMINISTRATION)
			.'</h1>'."\n");
	if(($configs = _config_list('probe')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'probe';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title host">'._html_safe(HOST_LIST)."</h2>\n");
	$module_id = _module_id('probe');
	$hosts = _sql_array('SELECT content_id AS id, title AS name, enabled'
			.' FROM daportal_content'
			." WHERE module_id='$module_id'");
	if(!is_array($hosts))
		return _error('Could not list hosts');
	for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
	{
		$hosts[$i]['module'] = 'probe';
		$hosts[$i]['action'] = 'host_modify';
		$hosts[$i]['icon'] = 'icons/16x16/host.png';
		$hosts[$i]['thumbnail'] = 'icons/48x48/host.png';
		$hosts[$i]['apply_module'] = 'probe';
		$hosts[$i]['apply_id'] = $hosts[$i]['id'];
		$hosts[$i]['name'] = _html_safe($hosts[$i]['name']);
		$hosts[$i]['enabled'] = $hosts[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$hosts[$i]['enabled'] = '<img src="icons/16x16/'
			.$hosts[$i]['enabled'].'.png" alt="'
			.$hosts[$i]['enabled'].'" title="'
			.($hosts[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_HOST, 'class' => 'new',
			'link' => _module_link('probe', 'host_new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'host_delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $hosts,
			'class' => array('enabled' => ENABLED),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'probe', 'action' => 'admin'));
}


function probe_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return probe_admin(array());
}


function probe_default($args)
{
	global $probe_types;

	if(!isset($args['id']) && !isset($args['type']))
	{
		probe_host_list(array());
		probe_graph_list(array());
		return;
	}
	$hosts = _host_list();
	if(!is_array($hosts))
		return _error('Could not list hosts');
	$title = MONITORING.':';
	if(isset($args['id']))
		foreach($hosts as $h)
			if($h['id'] == $args['id'])
			{
				$id = $args['id'];
				$title.=' '.$h['name'];
				break;
			}
	if(isset($args['type'])
			&& array_key_exists($args['type'], $probe_types))
	{
		$type = $args['type'];
		$title.=' '.$probe_types[$type]['name'];
	}
	$times = array('hour', 'day', 'week');
	$toolbar = _probe_toolbar();
	$action = 'default';
	if(isset($args['time']) && in_array($args['time'], $times))
		$time = $args['time'];
	include('./modules/probe/top.tpl');
	$keys = array_keys($probe_types);
	foreach($keys as $k)
	{
		$t = &$probe_types[$k];
		if(isset($type) && $type != $k)
			continue;
		for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
		{
			if(isset($id) && $hosts[$i]['id'] != $id)
				continue;
			foreach($times as $u)
			{
				if(isset($time) && $time != $u)
					continue;
				if(!isset($t['params']))
				{
					$graph = _host_graph($hosts[$i]['id'],
							$k, $u);
					include('./modules/probe/graph.tpl');
					continue;
				}
				foreach($t['params'] as $p)
				{
					$graph = _host_graph($hosts[$i]['id'],
							$k, $u, $p);
					if(!is_array($graph))
						continue;
					include('./modules/probe/graph.tpl');
				}
			}
		}
	}
	include('./modules/probe/bottom.tpl');
}


function probe_graph_list($args)
{
	global $probe_types;

	$title = GRAPH_LIST;
	include('./modules/probe/top.tpl');
	$types = array();
	$keys = array_keys($probe_types);
	foreach($keys as $k)
	{
		$t = $probe_types[$k];
		$t['module'] = 'probe';
		$t['action'] = 'default';
		$t['name'] = ucfirst($t['name']);
		$t['title'] = $t['name'];
		$t['args'] = '&type='.$k;
		if(!isset($t['thumbnail']))
			$t['thumbnail'] = 'icons/48x48/host.png';
		if(!isset($t['icon']))
			$t['icon'] = 'icons/16x16/host.png';
		$types[] = $t;
	}
	_module('explorer', 'browse', array('entries' => $types));
	include('./modules/probe/bottom.tpl');
}


function probe_host_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	_content_delete($args['id']);
}


function probe_host_insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(($id = _content_insert($args['hostname'], $args['comment'], 1))
			== FALSE)
		return _error('Could not insert host');
	probe_default(array('id' => $id));
}


function probe_host_list($args)
{
	$title = HOST_LIST;
	include('./modules/probe/top.tpl');
	$hosts = _host_list();
	if(!is_array($hosts))
		return _error('Could not list hosts');
	for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
	{
		$hosts[$i]['module'] = 'probe';
		$hosts[$i]['action'] = 'default';
		$hosts[$i]['icon'] = 'icons/16x16/host.png';
		$hosts[$i]['thumbnail'] = 'icons/48x48/host.png';
	}
	_module('explorer', 'browse', array('entries' => $hosts,
			'view' => 'list'));
	include('./modules/probe/bottom.tpl');
}


function probe_host_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$module_id = _module_id('probe');
	$host = _sql_array('SELECT content_id AS id, title AS hostname'
			.', content AS comment'.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."'"
			." AND enabled='1'");
	if(!is_array($host) || count($host) != 1)
		return _error('Could not modify host');
	$host = $host[0];
	$title = 'Host: '.$host['hostname'];
	include('./modules/probe/host_update.tpl');
}


function probe_host_new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = NEW_HOST;
	include('./modules/probe/host_update.tpl');
}


function probe_host_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	_content_update($args['id'], $args['hostname'], $args['comment']);
	probe_default(array('id' => $args['id']));
}


function probe_system($args)
{
	global $title, $error;

	$title.=' - '.MONITORING;
	$action = isset($args['action']) ? $args['action'] : 'default';
	switch($action)
	{
		case 'config_update':
			$error = _system_config_update($args);
			break;
		case 'default':
			if(isset($args['id']) || isset($args['type']))
				header('Refresh: 30');
			break;
	}
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	require_once('./system/config.php');
	_config_update('probe', $args);
	header('Location: '._module_link('probe', 'admin'));
	exit(0);
}

?>
