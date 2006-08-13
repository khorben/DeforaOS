<?php //modules/top/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function top_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="top">Top links administration</h1>'."\n");
	$links = _sql_array('SELECT top_id, name, link AS url FROM daportal_top'
			.' ORDER BY top_id ASC;');
	if(!is_array($links))
		return _error('Unable to get links');
	$count = count($links);
	$last_id = 0;
	for($i = 0; $i < $count; $i++)
	{
		/* $links[$i]['icon'] = ereg('^http://[^/]+/$',
				$links[$i]['url'])
				? _html_safe_link($links[$i]['url'])
						.'favicon.png'
				: 'modules/top/icon.png'; */
		$links[$i]['icon'] = 'modules/top/icon.png';
		$links[$i]['thumbnail'] = 'modules/top/icon.png';
		$links[$i]['apply_module'] = 'top';
		$links[$i]['apply_id'] = $links[$i]['top_id'];
		$links[$i]['link'] = 'index.php?module=top&action=modify&id='
				.$links[$i]['top_id'];
		$links[$i]['url'] = '<a href="'
				._html_safe_link($links[$i]['url'])
				.'">'._html_safe($links[$i]['url']).'</a>';
		$links[$i]['move'] = '';
		if($i+1 < $count)
			$links[$i]['move'].='<a href="index.php?module=top'
				.'&action=move&id='.$links[$i]['top_id']
				.'&to='.$links[$i+1]['top_id'].'">'
				.'<img src="modules/top/down.png" alt="down"/>'
				.'</a>';
		if($last_id)
			$links[$i]['move'].= '<a href="index.php?module=top'
				.'&action=move&id='.$links[$i]['top_id']
				.'&to='.$last_id.'">'
				.'<img src="modules/top/up.png" alt="up"/></a>';
		$last_id = $links[$i]['top_id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New link',
			'icon' => 'modules/top/icon.png',
			'link' => 'index.php?module=top&action=new');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE,
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete',
			'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array(
			'module' => 'top',
			'action' => 'admin',
			'view' => 'details',
			'class' => array('url' => 'Address', 'move' => 'Move'),
			'toolbar' => $toolbar,
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
		print("\t\t\t".$sep.'<a href="'._html_safe_link($l['link']).'">'
				._html_safe($l['name']).'</a>'."\n");
		$sep = '· ';
	}
	print("\t\t".'</div>'."\n");
}


function top_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	_sql_query('DELETE FROM daportal_top'
			." WHERE top_id='".$args['id']."';");
}


function top_insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query('INSERT INTO daportal_top (name, link) VALUES ('
			."'".$args['name']."', '".$args['link']."');") == FALSE)
		return _error('Unable to insert link');
	top_admin(array());
}


function top_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$top = _sql_array('SELECT top_id, name, link FROM daportal_top'
			." WHERE top_id='".$args['id']."';");
	if(!is_array($top) || count($top) != 1)
		return _error('Invalid link ID');
	$top = $top[0];
	$title = 'Top link modification';
	$action = 'update';
	include('./modules/top/update.tpl');
}


function top_move($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$from = _sql_array('SELECT name, link FROM daportal_top'
			." WHERE top_id='".$args['id']."';");
	$to = _sql_array('SELECT name, link FROM daportal_top'
			." WHERE top_id='".$args['to']."';");
	if(!is_array($from) || count($from) != 1 || !is_array($to)
			|| count($to) != 1)
		return _error('Unable to move links');
	$from = $from[0];
	$to = $to[0];
	if(!_sql_query("UPDATE daportal_top SET name='".$from['name']."'"
			.", link='".$from['link']."'"
			." WHERE top_id='".$args['to']."';")
			|| !_sql_query('UPDATE daportal_top'
					." SET name='".$to['name']."'"
					.", link='".$to['link']."'"
					." WHERE top_id='".$args['id']."';"))
		_error('Error while moving links');
	top_admin(array());
}


function top_new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = 'New top link';
	$action = 'insert';
	include('./modules/top/update.tpl');
}


function top_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!_sql_query('UPDATE daportal_top SET'
			." name='".$args['name']."'"
			.", link='".$args['link']."'"
			." WHERE top_id='".$args['id']."';"))
		return _error('Unable to update link');
	top_admin(array());
}

?>
