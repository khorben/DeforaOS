<?php //modules/content/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function content_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return content_modify(array('id' => $args['id']));
	print('<h1 class="title content">Content administration</h1>'."\n");
	$contents = _sql_array('SELECT content_id AS id, timestamp AS date'
			.', name AS module, username, title AS name'
			.', daportal_content.enabled AS enabled'
			.' FROM daportal_content, daportal_module'
			.', daportal_user'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' ORDER BY timestamp DESC;');
	if(!is_array($contents))
		return _error('Could not list contents');
	$count = count($contents);
	for($i = 0; $i < $count; $i++)
	{
		$contents[$i]['icon'] = '';
		if(($d = _module_desktop($contents[$i]['module'])) != FALSE)
			$contents[$i]['icon'] = $d['icon'];
		$contents[$i]['thumbnail'] = $contents[$i]['icon'];
		$contents[$i]['name'] = _html_safe_link($contents[$i]['name']);
		$contents[$i]['username'] = _html_safe($contents[$i]['username']);
		$contents[$i]['module'] = 'content';
		$contents[$i]['apply_module'] = 'content';
		$contents[$i]['action'] = 'admin';
		$contents[$i]['apply_id'] = $contents[$i]['id'];
		$contents[$i]['enabled'] = ($contents[$i]['enabled']
				== SQL_TRUE) ? 'enabled' : 'disabled';
		$contents[$i]['enabled'] = '<img src="icons/16x16/'
				.$contents[$i]['enabled'].'.png" alt="'
				.$contents[$i]['enabled'].'" title="'
				.($contents[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED)
				.'"/>';
		$contents[$i]['date'] = substr($contents[$i]['date'], 0, 19);
		$contents[$i]['date'] = date('d/m/Y H:i',
				strtotime($contents[$i]['date']));
	}
	$toolbar = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $contents,
			'class' => array('enabled' => ENABLED,
				'username' => AUTHOR, 'date' => DATE),
			'view' => 'details', 'toolbar' => $toolbar,
			'module' => 'content', 'action' => 'admin'));
}


function content_default($args)
{
	global $user_id;

	if(!isset($args['id']))
	{
		return include('default.tpl');
	}
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$where = '';
	else
		$where = " AND daportal_content.enabled='1'";
	$content = _sql_array('SELECT name'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content'
			.'.module_id=daportal_module.module_id'
			.$where." AND content_id='".$args['id']."'");
	if(!is_array($content) || count($content) != 1)
		return _error('Could not display content');
	$content = $content[0];
	_module($content['name'], 'default', array('id' => $args['id']));
}


function content_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query('DELETE FROM daportal_content WHERE'
			." content_id='".$args['id']."'") == FALSE)
		_error('Unable to delete content');
}


function content_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(_content_disable($args['id']) == FALSE)
		_error('Unable to update content');
	if(isset($args['show']))
		content_default(array('id' => $args['id']));
}


function content_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(_content_enable($args['id']) == FALSE)
		_error('Unable to update content');
	if(isset($args['show']))
		content_default(array('id' => $args['id']));
}


function content_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	if(!is_numeric($id))
		return _error('Invalid content');
	$content = _sql_array('SELECT timestamp, title, content, enabled'
			.' FROM daportal_content'
			." WHERE content_id='$id'");
	if(!is_array($content) || count($content) != 1)
		return _error('Invalid content');
	$content = $content[0];
	include('update.tpl');
}


function content_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query('UPDATE daportal_content SET '
			." title='".$args['title']."'"
			.", timestamp='".$args['timestamp']."'"
			.", content='".$args['content']."'"
			.", enabled='".$args['enabled']."'"
			." WHERE content_id='".$args['id']."'") == FALSE)
		_error('Unable to update content');
	content_modify(array('id' => $args['id']));
}

?>
