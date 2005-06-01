<?php
//modules/admin/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));
require_once('system/user.php');


function _content_modify($id)
{
	if(!is_numeric($id))
		return _error('Invalid content');
	$content = _sql_array('SELECT timestamp, title, content, enabled'
			.' FROM daportal_content'
			." WHERE content_id='$id';");
	if(!is_array($content) || count($content) != 1)
		return _error('Invalid content');
	$content = $content[0];
	include('content_update.tpl');
}


function admin_content($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(isset($args['id']))
		return _content_modify($args['id']);
	print('<h1><img src="modules/admin/icon.png" alt=""/> Contents administration</h1>'."\n");
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
		$contents[$i]['icon'] = 'modules/'.$contents[$i]['module']
				.'/icon.png';
		$contents[$i]['thumbnail'] = $contents[$i]['icon'];
		$contents[$i]['name'] = _html_safe_link($contents[$i]['name']);
		$contents[$i]['module'] = 'admin';
		$contents[$i]['action'] = 'content';
		$contents[$i]['enabled'] = ($contents[$i]['enabled'] == 't')
				? 'enabled' : 'disabled';
		$contents[$i]['enabled'] = '<img src="modules/admin/'
				.$contents[$i]['enabled'].'" alt="'
				.$contents[$i]['enabled'].'"/>';
/*		$contents[$i]['date'] = date('l, F jS Y, H:i',
				strtotime($contents[$i]['date'])); */
	}
	_module('explorer', 'browse_trusted', array(
			'class' => array('enabled' => '', 'date' => 'Date'),
			'entries' => $contents,
			'view' => 'details'));
}


function admin_content_update($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(_sql_query('UPDATE daportal_content SET '
			." title='".$args['title']."'"
			.", timestamp='".$args['timestamp']."'"
			.", content='".$args['content']."'"
			.", enabled='".$args['enabled']."'"
			." WHERE content_id='".$args['id']."';") == FALSE)
		_error('Unable to update content');
	_content_modify($args['id']);
}


function admin_default()
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!_sql_single('SELECT admin FROM daportal_user'
			." WHERE user_id='$user_id';"))
		return error('Permission denied');
	include('default.tpl');
}


function admin_module($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(isset($args['id']))
		return _module_admin($args['id']);
	print('<h1><img src="modules/admin/icon.png" alt=""/> Modules administration</h1>'."\n");
	if(($modules = _sql_array('SELECT module_id, name AS module'
			.', enabled'
			.' FROM daportal_module'
			.' ORDER BY enabled DESC, module ASC;')) == FALSE)
		return _error('Could not list modules');
	$count = count($modules);
	for($i = 0; $i < $count; $i++)
	{
		$module = $modules[$i]['module'];
		$modules[$i]['icon'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['thumbnail'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['action'] = 'admin';
		$modules[$i]['module_name'] = '<a href="index.php?module='
				._html_safe_link($module).'">'
				._html_safe($module).'</a>';
		$title = '';
		@include('modules/'.$module.'/desktop.php');
		$modules[$i]['name'] = _html_safe_link(strlen($title) ? $title
				: $modules[$i]['module']);
	}
	_module('explorer', 'browse_trusted', array(
			'class' => array('module_name' => 'Module name'),
			'entries' => $modules));
}


function admin_site($args)
{
	//FIXME remember exactly what I wanted to do here:
	//- virtual hosts? with database switching?
	//- ...?
	print('<h1><img src="modules/admin/icon.png" alt=""/> Sites administration</h1>'."\n");
}

?>
