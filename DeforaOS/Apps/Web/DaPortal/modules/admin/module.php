<?php
//modules/admin/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));
require_once('system/user.php');


//lang
$text['GLOBAL_CONFIGURATION'] = 'Global configuration';
$text['MODULE_NAME'] = 'Module name';
$text['MODULES_ADMINISTRATION'] = 'Modules administration';
global $lang;
if($lang == 'fr')
{
	$text['MODULE_NAME'] = 'Nom du module';
	$text['MODULES_ADMINISTRATION'] = 'Administration des modules';
}
_lang($text);


function admin_admin($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	include('admin.tpl');
}


/* config */
function admin_config($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/admin/icon.png" alt=""/> '
			.GLOBAL_CONFIGURATION.'</h1>'."\n");
	$keys = array_keys($args);
	foreach($keys as $k)
	{
		if(!ereg('^([a-zA-Z]+)_([a-zA-Z_]+)$', $k, $regs))
			continue;
		_config_set($regs[1], $regs[2], $args[$k], 0);
	}
	$configs = _sql_array('SELECT daportal_module.name AS module'
			.', daportal_config.name AS name, value'
			.' FROM daportal_config, daportal_module'
			.' WHERE daportal_config.module_id'
			.'=daportal_module.module_id'
			.' ORDER BY daportal_module.name ASC'
			.', daportal_config.name ASC;');
	include('config_update.tpl');
}


function admin_default()
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$modules = _sql_array('SELECT module_id, name FROM daportal_module'
			.' ORDER BY NAME ASC;');
	for($i = 0, $cnt = count($modules); $i < $cnt; $i++)
	{
		$admin = 0;
		$title = '';
		@include('modules/'.$modules[$i]['name'].'/desktop.php');
		$modules[$i]['admin'] = $admin;
		$modules[$i]['title'] = $title;
	}
	include('default.tpl');
}


/* module */
function admin_module($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return _module_admin($args['id']);
	print('<h1><img src="modules/admin/icon.png" alt=""/> '
			._html_safe(MODULES_ADMINISTRATION)
			.'</h1>'."\n");
	if(($modules = _sql_array('SELECT module_id AS apply_id, name AS module'
			.', enabled'
			.' FROM daportal_module'
			.' ORDER BY module ASC;')) == FALSE)
		return _error('Could not list modules');
	$count = count($modules);
	for($i = 0; $i < $count; $i++)
	{
		$module = $modules[$i]['module'];
		$modules[$i]['icon'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['thumbnail'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['action'] = 'admin';
		$modules[$i]['apply_module'] = 'admin';
		$modules[$i]['enabled'] = ($modules[$i]['enabled'] == 't')
				? 'enabled' : 'disabled';
		$modules[$i]['enabled'] = '<img src="icons/16x16/'
				.$modules[$i]['enabled'].'" alt="'
				.$modules[$i]['enabled'].'" title="'
				.($modules[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED)
				.'"/>';
		$modules[$i]['module_name'] = '<a href="index.php?module='
				._html_safe_link($module).'">'
				._html_safe($module).'</a>';
		$title = '';
		@include('modules/'.$module.'/desktop.php');
		$modules[$i]['name'] = _html_safe_link(strlen($title) ? $title
				: $modules[$i]['module']);
	}
	$toolbar = array();
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'module_disable');
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'module_enable');
	_module('explorer', 'browse_trusted', array(
			'class' => array('enabled' => ENABLED,
					'module_name' => MODULE_NAME),
			'entries' => $modules,
			'view' => 'details',
			'toolbar' => $toolbar,
			'module' => 'admin',
			'action' => 'module'));
}


function admin_module_disable($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='f'"
			." WHERE module_id='".$args['id']."';") == FALSE)
		_error('Unable to update module');
}


function admin_module_enable($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='t'"
			." WHERE module_id='".$args['id']."';") == FALSE)
		_error('Unable to update module');
}


/* site */
function admin_site($args)
{
	//FIXME remember exactly what I wanted to do here:
	//- virtual hosts? with database switching?
	//- ...?
	print('<h1><img src="modules/admin/icon.png" alt=""/> Sites administration</h1>'."\n");
}


function admin_system($args)
{
	global $title;

	$title.=' - Administration';
}

?>
