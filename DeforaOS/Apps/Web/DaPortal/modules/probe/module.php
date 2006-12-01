<?php //modules/probe/module.php
//TODO:
//- fix configuration when name has a space



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['CONFIGURATION_ERROR'] = 'Configuration error';
$text['HOST_LIST'] = 'Host list';
$text['MONITORING'] = 'Monitoring';
$text['MONITORING_ADMINISTRATION'] = 'Monitoring administration';
global $lang;
if($lang == 'fr')
{
	$text['CONFIGURATION_ERROR'] = 'Erreur de configuration';
	$text['HOST_LIST'] = 'Liste des machines';
	$text['MONITORING'] = 'Suivi';
	$text['MONITORING_ADMINISTRATION'] = 'Administration du suivi';
}
_lang($text);


//private
function _host_graph($hostname, $graph, $time, $param)
{
	if(!($probe = _config_get('probe', 'RRD_repository')))
		return _error(CONFIGURATION_ERROR);
	//FIXME not always true (if, vol)
	$rrd = $probe.'/'.$hostname.'_'.$graph.'.rrd';
	_info('rrd: '.$rrd);
	switch($time)
	{
		case 'week':	$start = '-604800';	break;
		case 'day':	$start = '-86400';	break;
		case 'hour':
		default:	$start = '-3600';
				$time = 'hour';		break;
	}
	//FIXME hardcoded path + not always true (if, vol)
	$png = 'tmp/'.$hostname.'_'.$graph.'_'.$time.'.png';
	if(($st = @stat($png)) != FALSE && $st['mtime'] + 30 > time())
		return $png;
	$title = '';
	$label = '';
	$base = '';
	$def = array();
	$cdef = array();
	$data = '';
	$args = '-l 0';
	switch($graph)
	{
		case 'uptime':
			$title = 'uptime';
			$label = 'hours';
			$def = array('uptime');
			$cdef = array('ruptime' => 'uptime,3600,/');
			$data = ' AREA:ruptime#ffc0c0'
				.' LINE1:ruptime#ef0f0f:"Uptime\t\g"'
				.' GPRINT:ruptime:LAST:"Current\: %.0lf h\t\g"'
				.' GPRINT:ruptime:AVERAGE:"Average\: %.0lf h\t\g"'
				.' GPRINT:ruptime:MAX:"Maximum\: %.0lf h"';
			break;
		case 'load':
			$title = 'load average';
			$label = 'load';
			$def = array('load1', 'load5', 'load15');
			$cdef = array('rload1' => 'load1,65536,/',
				'rload5' => 'load15,65536,/',
				'rload15' => 'load15,65536,/');
			$data = ' AREA:rload1#ffef5f'
				.' AREA:rload5#ffbf5f'
				.' AREA:rload15#ff8f5f'
				.' LINE1:rload1#ffdf00:"Load 1 min\t\g"'
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
				.' GPRINT:rload15:MAX:"Maximum\: %.2lf"';
			break;
		case 'ram':
			$title = 'memory usage';
			$label = 'bytes';
			$base = '1024';
			$def = array('ramtotal', 'ramfree', 'ramshared',
					'rambuffer');
			$cdef = array('pramtotal' => 'ramtotal,1024,/,1024,/',
				'pramfree' => 'ramfree,1024,/,1024,/',
				'pramshared' => 'ramshared,1024,/,1024,/',
				'prambuffer' => 'rambuffer,1024,/,1024,/');
			$data = ' AREA:ramtotal#ff0000:"Total   "'
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
				.' GPRINT:prambuffer:LAST:"Maximum\: %.0lf MB\g"';
			break;
		case 'swap':
			$title = 'swap usage';
			$label = 'bytes';
			$base = '1024';
			$def = array('swaptotal', 'swapfree');
			$cdef = array('pswaptotal' => 'swaptotal,1024,/,1024,/',
				'pswapfree' => 'swapfree,1024,/,1024,/');
			$data = ' AREA:swaptotal#ff0000:"Total\t\g"'
				.' GPRINT:pswaptotal:LAST:"Current\: %.0lf MB\t\g"'
				.' GPRINT:pswaptotal:AVERAGE:"Average\: %.0lf MB\t\g"'
				.' GPRINT:pswaptotal:MAX:"Maximum\: %.0lf MB\n"'
				.' AREA:swapfree#0000ff:"Free\t\g"'
				.' GPRINT:pswapfree:LAST:"Current\: %.0lf MB\t\g"'
				.' GPRINT:pswapfree:AVERAGE:"Average\: %.0lf MB\t\g"'
				.' GPRINT:pswapfree:MAX:"Maximum\: %.0lf MB\g"';
			break;
		case 'users':
			$title = 'logged users';
			$label = 'users';
			$def = array('users');
			$data = ' AREA:users#b0b0ff'
				.' LINE1:users#0f0fef:"Logged users\t\g"'
				.' GPRINT:users:LAST:"Current\: %.0lf\t\g"'
				.' GPRINT:users:AVERAGE:"Average\: %.0lf\t\g"'
				.' GPRINT:users:MAX:"Maximum\: %.0lf"';
			break;
		case 'procs':
			$title = 'process count';
			$label = 'process';
			$def = array('procs');
			$data = ' AREA:procs#b0b0ff'
				.' LINE1:procs#0f0fef:"Process count\t\g"'
				.' GPRINT:procs:LAST:"Current\: %.0lf \t\g"'
				.' GPRINT:procs:AVERAGE:"Average\: %.0lf \t\g"'
				.' GPRINT:procs:MAX:"Maximum\: %.0lf"';
			break;
		case 'iface':
			$rrd = $probe.'/'.$hostname.'_'.$param.'.rrd';
			//FIXME hardcoded path
			$png = 'tmp/'.$hostname.'_'.$param.'_'.$time.'.png';
			$title = 'network traffic on '.$param;
			$label = 'Bps';
			$def = array('ifrxbytes', 'iftxbytes');
			$cdef = array('ifrxkb' => 'ifrxbytes,1024,/',
					'iftxkb' => 'iftxbytes,1024,/');
			$data = ' AREA:ifrxbytes#b0ffb0'
				.' LINE1:ifrxbytes#0fef0f:"RX\t\g"'
				.' GPRINT:ifrxkb:LAST:"Current\: %.0lf KBps\t\g"'
				.' GPRINT:ifrxkb:AVERAGE:"Average\: %.0lf KBps\t\g"'
				.' GPRINT:ifrxkb:MAX:"Maximum\: %.0lf KBps\n"'
				.' LINE1:iftxbytes#0f0fef:"TX\t\g"'
				.' GPRINT:iftxkb:LAST:"Current\: %.0lf KBps\t\g"'
				.' GPRINT:iftxkb:AVERAGE:"Average\: %.0lf KBps\t\g"'
				.' GPRINT:iftxkb:MAX:"Maximum\: %.0lf KBps\g"';
			break;
		case 'vol':
			$rrd = $probe.'/'.$hostname.'/'.$param.'.rrd';
			//FIXME hardcoded path
			if(!is_dir('tmp/'.$hostname))
				mkdir('tmp/'.$hostname);
			$png = 'tmp/'.$hostname.'/'.$param.'_'.$time.'.png';
			$title = 'volume usage: '.$param;
			$label = 'MB';
			$def = array('voltotal', 'volfree');
			//FIXME block size may not be 4
			$cdef = array('pvoltotal' => 'voltotal,1024,/',
				'pvolfree' => 'volfree,1024,/');
			$data = ' AREA:pvoltotal#ff0000:"Total\:\g"'
				.' GPRINT:pvoltotal:LAST:" %.0lf MB"'
				.' AREA:pvolfree#0000ff:"Free\:\g"'
				.' GPRINT:pvolfree:LAST:" %.0lf MB"';
			break;
		default:
			_error('Unknown graph to update');
			return '';
	}
	$cmd = 'rrdtool graph --slope-mode '.$png
		.' --start '.$start.' --imgformat PNG'
		.' -c BACK#dcdad5 -c SHADEA#ffffff -c SHADEB#9e9a91';
	if(strlen($base))
		$cmd.=' --base '.$base;
	foreach($def as $d)
		$cmd.=' DEF:'.$d.'='.$rrd.':'.$d.':AVERAGE';
	$keys = array_keys($cdef);
	foreach($keys as $k)
		$cmd.=' CDEF:'.$k.'='.$cdef[$k];
	$cmd.=$data;
	if(strlen($args))
		$cmd.=' '.$args;
	$cmd.=' --title "'.$hostname.' '.$title.' (last '.$time.')"'
		.' --vertical-label "'.$label.'"';
	_info('exec: '.$cmd);
	//FIXME check potential command insertion through hostname/title/etc
	exec($cmd);
	return $png;
}


function probe_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	print('<h1 class="title probe">'._html_safe(MONITORING_ADMINISTRATION)
			.'</h1>'."\n");
	if(($configs = _config_list('probe')))
	{
		print('<h2><img src="modules/probe/icon.png" alt=""/> '
				.'Configuration</h2>'."\n");
		$module = 'probe';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2><img src="modules/probe/icon.png" alt=""/> '
			._html_safe(HOST_LIST)."</h2>\n");
	$hosts = _sql_array('SELECT host_id AS id, title AS name, enabled'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id;');
	if(!is_array($hosts))
		return _error('Could not list hosts');
	for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
	{
		$hosts[$i]['module'] = 'probe';
		$hosts[$i]['action'] = 'host_modify';
		$hosts[$i]['icon'] = 'modules/probe/icon.png';
		$hosts[$i]['thumbnail'] = 'modules/probe/icon.png';
		$hosts[$i]['apply_module'] = 'probe';
		$hosts[$i]['apply_id'] = $hosts[$i]['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New host',
			'icon' => 'modules/probe/icon.png',
			'link' => 'index.php?module=probe&action=host_new');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE,
			'icon' => 'icons/16x16/delete.png',
			'action' => 'host_delete', 'confirm' => 'delete');
	_module('explorer', 'browse', array('entries' => $hosts,
			'toolbar' => $toolbar,
			'module' => 'probe', 'action' => 'admin'));
}


function probe_config_update($args)
{
	global $user_id, $module_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/config.php');
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^probe_([a-zA-Z_]+)$', $k, $regs))
			_config_set('probe', $regs[1], $args[$k], 0);
	header('Location: index.php?module=probe&action=admin');
	exit(0);
}


function probe_default($args)
{
	if(isset($args['id']))
		return probe_host_display(array('id' => $args['id']));
	return probe_host_list(array());
}


function probe_host_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	_sql_query('DELETE FROM daportal_probe_host'
			." WHERE host_id='".$args['id']."';");
	_sql_query('DELETE FROM daportal_content'
			." WHERE content_id='".$args['id']."';");
}


function probe_host_display($args)
{
	if(!($probe = _config_get('probe', 'RRD_repository')))
		return _error(CONFIGURATION_ERROR);
	$host = _sql_array('SELECT host_id AS id, title AS hostname'
			.', content AS comment'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id'
			." AND enabled='1'"
			." AND host_id='".$args['id']."';");
	if(!is_array($host) || count($host) != 1)
		return _error('Could not display host');
	$time = 'hour';
	if(isset($args['time']))
		switch($args['time'])
		{
			case 'day':	$time = 'day';	break;
			case 'week':	$time = 'week';	break;
		}
	$host = $host[0];
	$title = 'Host: '.$host['hostname'];
	//FIXME graphs, categories, ...
	$graphs = array();
	$graphs[] = array('graph' => 'uptime', 'title' => 'Uptime',
			'time' => $time);
	$graphs[] = array('graph' => 'load', 'title' => 'Load average',
			'time' => $time);
	$graphs[] = array('graph' => 'ram', 'title' => 'Memory usage',
			'time' => $time);
	$graphs[] = array('graph' => 'swap', 'title' => 'Swap usage',
			'time' => $time);
	$graphs[] = array('graph' => 'users', 'title' => 'Logged users',
			'time' => $time);
	$graphs[] = array('graph' => 'procs', 'title' => 'Process count',
			'time' => $time);
	//FIXME potential directory traversal, ugly enumeration workaround
	foreach(array('eth0', 'eth1', 'eth2', 'eth3', 'ppp0', 'pppoe0', 'sip0',
				'sip1', 'sip2', 'sip3', 'sip4', 'ex0') as $if)
		if(is_readable($probe.'/'.$host['hostname']."_$if.rrd"))
			$graphs[] = array('graph' => 'iface',
					'title' => 'Network usage: '.$if,
					'time' => $time, 'param' => $if);
	//FIXME directory traversal + hidden rrd (*/.rrd)
	$vols = glob($probe.'/'.$host['hostname'].'/*.rrd');
	foreach($vols as $v)
	{
		$v = substr($v, strlen($probe) + 1 + strlen($host['hostname']));
		$v = substr($v, 0, -4);
		$graphs[] = array('graph' => 'vol',
				'title' => 'Volume usage: '.$v, 'time' => $time,
				'param' => $v);
	}
	if(isset($args['graph']))
		foreach($graphs as $g)
			if($g['graph'] == $args['graph'])
			{
				$graph = $args['graph'];
				break;
			}
	include('./modules/probe/host_display.tpl');
}


function probe_host_insert($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	require_once('system/content.php');
	if(($id = _content_insert($args['hostname'], $args['comment'], 1))
			== FALSE)
		return _error('Could not insert host');
	_sql_query('INSERT INTO daportal_probe_host (host_id) VALUES ('
			."'$id'".');');
	probe_host_display(array('id' => $id));
}


function probe_host_list($args)
{
	print('<h1 class="title probe">'._html_safe(HOST_LIST).'</h1>'."\n");
	//FIXME sort and display by category
	$hosts = _sql_array('SELECT host_id AS id, title AS name'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id'
			." AND enabled='1' ORDER BY id ASC;");
	if(!is_array($hosts))
		return _error('Could not list hosts');
	for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
	{
		$hosts[$i]['module'] = 'probe';
		$hosts[$i]['action'] = 'host_display';
		$hosts[$i]['icon'] = 'modules/probe/icon.png';
		$hosts[$i]['thumbnail'] = 'modules/probe/icon.png';
	}
	_module('explorer', 'browse', array('entries' => $hosts,
			'view' => 'thumbnails', 'toolbar' => 0));
}


function probe_host_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	$host = _sql_array('SELECT host_id AS id, title AS hostname'
			.', content AS comment'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id'
			." AND enabled='1'"
			." AND host_id='".$args['id']."';");
	if(!is_array($host) || count($host) != 1)
		return _error('Could not modify host');
	$host = $host[0];
	$title = 'Host: '.$host['hostname'];
	//FIXME graphs, categories, ...
	include('./modules/probe/host_update.tpl');
}


function probe_host_new($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	$title = 'New host';
	include('./modules/probe/host_update.tpl');
}


function probe_host_update($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	require_once('system/content.php');
	_content_update($args['id'], $args['hostname'], $args['comment']);
	probe_host_display(array('id' => $args['id']));
}


function probe_system($args)
{
	global $title, $html;

	$title.=' - '.MONITORING;
	if(!isset($args['action']))
		return;
	switch($args['action'])
	{
		case 'config_update':
			$html = 0;
			break;
		case 'host_display':
			header('refresh: 30');
			break;
	}
}

?>
