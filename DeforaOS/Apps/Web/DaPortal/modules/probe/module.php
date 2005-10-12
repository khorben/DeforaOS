<?php
//modules/probe/module.php
//TODO:
//- on-demand RRD graphs creation? (optional?)



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


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
			'icon' => 'modules/probe/host.png',
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
	/* FIXME sort and display by category */
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
	_module('explorer', 'browse', array(
			'entries' => $hosts,
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
	/* FIXME graphs, categories, ... */
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
