<?php
//modules/project/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function project_default($args)
{
	if(isset($args['id']))
		return project_display($args);
	include('default.tpl');
}


function project_display($args)
{
	global $user_id;

	require_once('system/user.php');
	$project = _sql_array('SELECT name, title, content AS description'
			.', enabled'
			.' FROM daportal_content, daportal_project'
			." WHERE content_id='".$args['id']."'"
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id;');
	if(!is_array($project) || count($project) != 1)
		return _error('Unable to display project');
	$project = $project[0];
	if($project['enabled'] != 't' && !_user_admin($user_id))
		return include('project_submitted.tpl');
	$title = $project['name'];
	include('project_display.tpl');
}


function project_download($args)
{
	include('download.tpl');
}


function project_insert($args)
{
	require_once('system/content.php');
	if(($id = _content_insert($args['title'], $args['content'])) == FALSE)
		return _error('Unable to insert project content');
	if(!_sql_query('INSERT INTO daportal_project (project_id, name) VALUES'
			."('$id', '".$args['name']."');"))
		return _error('Unable to insert project');
	project_display(array('id' => $id));
}


function project_installer($args)
{
	include('installer.tpl');
}


function project_list($args)
{
	global $module_id;

	$title = 'Projects list';
	$level = 1;
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			." FROM daportal_user WHERE user_id='".$args['user_id']
			."';")) != FALSE)
	{
		$title = 'Projects by '.$username;
		$level = 2;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h'.$level.'><img src="modules/project/icon.png" alt=""/> '
			._html_safe($title).'</h'.$level.'>'."\n");
	$projects = _sql_array('SELECT content_id AS id, name, title AS desc'
			.', username AS admin'
			.' FROM daportal_content, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='1'"
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id'
			.$where
			.' ORDER BY name ASC;');
	if(!is_array($projects))
		return _error('Could not list projects');
	$count = count($projects);
	for($i = 0; $i < $count; $i++)
	{
		$projects[$i]['module'] = 'project';
		$projects[$i]['action'] = 'display';
		$projects[$i]['icon'] = 'modules/project/icon.png';
		$projects[$i]['thumbnail'] = 'modules/project/icon.png';
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New project',
			'icon' => 'modules/project/icon.png',
			'link' => 'index.php?module=project&action=new');
	_module('explorer', 'browse', array(
			'class' => array('admin' => 'Administrator',
					'desc' => 'Description'),
			'toolbar' => $toolbar,
			'view' => 'details',
			'entries' => $projects));
}


function project_new($args)
{
	include('project_update.tpl');
}


function project_package($args)
{
	include('package.tpl');
}


function project_report($args)
{
	include('bug_display.tpl');
}

?>
