<?php //modules/probe/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//private
function _host_graph($hostname, $graph, $time)
{
	$rrd = '/var/lib/Probe/'.$hostname.'_'.$graph.'.rrd'; //FIXME
	_info('rrd: '.$rrd);
	switch($time)
	{
		case 'week':
			$start = '-604800';
			break;
		case 'day':
			$start = '-86400';
			break;
		case 'hour':
		default:
			$start = '-3600';
			$time = 'hour';
			break;
	}
	$png = 'tmp/'.$hostname.'_'.$graph.'_'.$time.'.png'; //FIXME
	if(($st = stat($png)) != FALSE && $st['mtime'] + 30 > time())
		return $png;
	$title = '';
	$label = '';
	$base = '';
	$def = array();
	$cdef = array();
	$data = '';
	switch($graph)
	{
		case 'uptime':
			$title = 'uptime';
			$label = 'hours';
			$def = array('uptime');
			$cdef = array('ruptime' => 'uptime,3600,/');
			$data = ' AREA:ruptime#ff7f7f'
				.' LINE2:ruptime#ff4f4f:"Uptime"'
				.' GPRINT:ruptime:LAST:" %.2lf h"';
			break;
		case 'load':
			$title = 'load average';
			$label = 'load';
			$def = array('load1', 'load5', 'load15');
			$cdef = array('rload1' => 'load1,65536,/',
				'rload5' => 'load15,65536,/',
				'rload15' => 'load15,65536,/');
			$data = ' AREA:rload1#ffef00'
				.' AREA:rload5#ffbf00'
				.' AREA:rload15#ff8f00'
				.' LINE2:rload1#ffdf00:"Load 1 min\:\g"'
				.' GPRINT:rload1:LAST:" %.2lf"'
				.' LINE2:rload5#ffaf00:"Load 5 min\:\g"'
				.' GPRINT:rload5:LAST:" %.2lf"'
				.' LINE2:rload15#ff7f00:"Load 15 min\:\g"'
				.' GPRINT:rload15:LAST:" %.2lf"';
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
			$data = ' AREA:ramtotal#ff0000:"Total\:\g"'
				.' GPRINT:pramtotal:LAST:" %.0lf MB"'
				.' AREA:ramfree#0000ff:"Free\:\g"'
				.' GPRINT:pramfree:LAST:" %.0lf MB"'
				.' STACK:ramshared#00ffff:"Shared\:\g"'
				.' GPRINT:pramshared:LAST:" %.0lf MB"'
				.' STACK:rambuffer#00ff00:"Buffer\:\g"'
				.' GPRINT:prambuffer:LAST:" %.0lf MB"';
			break;
		case 'swap':
			$title = 'swap usage';
			$label = 'bytes';
			$base = '1024';
			$def = array('swaptotal', 'swapfree');
			$cdef = array('pswaptotal' => 'swaptotal,1024,/,1024,/',
				'pswapfree' => 'swapfree,1024,/,1024,/');
			$data = ' AREA:swaptotal#ff0000:"Total\:\g"'
				.' GPRINT:pswaptotal:LAST:" %.0lf MB"'
				.' AREA:swapfree#0000ff:"Free\:\g"'
				.' GPRINT:pswapfree:LAST:" %.0lf MB"';
			break;
		case 'users':
			$title = 'logged users';
			$label = 'users';
			$def = array('users');
			$data = ' AREA:users#7f7fff'
				.' LINE2:users#4f4fff:"Logged users\:\g"'
				.' GPRINT:users:LAST:" %.0lf"';
			break;
		case 'procs':
			$title = 'process count';
			$label = 'process';
			$def = array('procs');
			$data = ' AREA:procs#7f7fff'
				.' LINE2:procs#4f4fff:"Process count\:\g"'
				.' GPRINT:procs:LAST:" %.0lf"';
			break;
		default:
			return ''; //FIXME
	}
	$cmd = 'rrdtool graph '.$png.' --start '.$start.' --imgformat PNG'
		.' -c BACK#dcdad5 -c SHADEA#ffffff -c SHADEB#9e9a91';
	if(strlen($base))
		$cmd.=' --base '.$base;
	foreach($def as $d)
		$cmd.=' DEF:'.$d.'='.$rrd.':'.$d.':AVERAGE';
	$keys = array_keys($cdef);
	foreach($keys as $k)
		$cmd.=' CDEF:'.$k.'='.$cdef[$k];
	$cmd.=$data;
	$cmd.=' --title "'.$hostname.' '.$title.' (last '.$time.')"'
		.' --vertical-label "'.$label.'"';
	_info('exec: '.$cmd);
	exec($cmd);
	return $png;
}


function probe_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	print('<h1><img src="modules/probe/icon.png" alt=""/>'
			.' Monitoring administration</h1>'."\n");
	$hosts = _sql_array('SELECT host_id AS id, title AS name, enabled'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id;');
	if(!is_array($hosts))
		return _error('Could not list hosts');
	for($i = 0, $cnt = count($hosts); $i < $cnt; $i++)
	{
		$hosts[$i]['module'] = 'probe';
		$hosts[$i]['action'] = 'host_display';
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
			'action' => 'host_delete',
			'confirm' => 'delete');
	_module('explorer', 'browse', array(
			'entries' => $hosts,
			'toolbar' => $toolbar,
			'module' => 'probe',
			'action' => 'admin'));
}


function probe_default($args)
{
	return probe_host_list(array());
}


function probe_host_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	_sql_query('DELETE FROM daportal_probe_host'
			." WHERE host_id='".$args['id']."';");
	_sql_query('DELETE FROM daportal_content'
			." WHERE content_id='".$args['id']."';");
}


function probe_host_display($args)
{
	$host = _sql_array('SELECT host_id AS id, title AS hostname'
			.', content AS comment'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id'
			." AND enabled='t'"
			." AND host_id='".$args['id']."';");
	if(!is_array($host) || count($host) != 1)
		return _error('Could not display host');
	$host = $host[0];
	$title = 'Host: '.$host['hostname'];
	/* FIXME graphs, categories, ... */
	include('host_display.tpl');
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
	print('<h1><img src="modules/probe/icon.png" alt=""/>'
			.' Hosts list</h1>'."\n");
	//FIXME sort and display by category
	$hosts = _sql_array('SELECT host_id AS id, title AS name'
			.' FROM daportal_probe_host, daportal_content'
			.' WHERE content_id=host_id'
			." AND enabled='t';");
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
			'view' => 'thumbnails',
			'toolbar' => 0));
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
			." AND enabled='t'"
			." AND host_id='".$args['id']."';");
	if(!is_array($host) || count($host) != 1)
		return _error('Could not modify host');
	$host = $host[0];
	$title = 'Host: '.$host['hostname'];
	//FIXME graphs, categories, ...
	include('host_update.tpl');
}


function probe_host_new($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	$title = 'New host';
	include('host_update.tpl');
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
	switch($args['action'])
	{
		case 'host_display':
			header('refresh: 30');
			break;
	}
}

?>
