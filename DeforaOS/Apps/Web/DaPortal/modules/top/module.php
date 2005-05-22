<?php
//modules/top/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function top_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	print('<h1>Top links administration</h1>'."\n");
	$links = _sql_array('SELECT top_id, name, link AS url FROM daportal_top'
			.' ORDER BY top_id ASC;');
	if(!is_array($links))
		return _error('Unable to get links');
	$count = count($links);
	for($i = 0; $i < $count; $i++)
	{
		$links[$i]['icon'] = 'modules/top/icon.png';
		$links[$i]['thumbnail'] = 'modules/top/icon.png';
		$links[$i]['link'] = 'index.php?module=top&action=modify&id='
				.$links[$i]['top_id'];
		$links[$i]['url'] = '<a href="'._html_safe($links[$i]['url'])
				.'">'._html_safe($links[$i]['url']).'</a>';
	}
	_module('explorer', 'browse_trusted', array(
			'view' => 'details',
			'class' => array('url' => 'Address'),
			'entries' => $links));
}


function top_default($args)
{
	$links = _sql_array('SELECT name, link FROM daportal_top'
			.' ORDER BY top_id ASC;');
	if(!is_array($links))
		return _error('Unable to get links');
	print("\t\t".'<div class="top">'."\n");
	$sep = '';
	foreach($links as $l)
	{
		print("\t\t\t".$sep.'<a href="'._html_safe($l['link']).'">'
				._html_safe($l['name']).'</a>'."\n");
		$sep = '· ';
	}
	print("\t\t".'</div>'."\n");
}


function top_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	$top = _sql_array('SELECT top_id, name, link FROM daportal_top'
			." WHERE top_id='".$args['id']."';");
	if(!is_array($top) || count($top) != 1)
		return _error('Invalid link ID');
	$top = $top[0];
	include('update.tpl');
}


function top_update($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!_sql_query('UPDATE daportal_top SET'
			." name='".$args['name']."'"
			.", link='".$args['link']."'"
			." WHERE top_id='".$args['id']."';"))
		return _error('Unable to update link');
	top_admin(array());
}

?>
