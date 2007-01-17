<?php //modules/admin/module.php



//check url
if(strcmp($_SERVER['SCRIPT_NAME'], $_SERVER['PHP_SELF']) != 0
		|| !ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


require_once('./system/user.php');


//lang
$text = array();
$text['ADMINISTRATION'] = 'Administration';
$text['GLOBAL_CONFIGURATION'] = 'Global configuration';
$text['LANGUAGES'] = 'Languages';
$text['PORTAL_ADMINISTRATION'] = 'Portal administration';
$text['MODULE_NAME'] = 'Module name';
$text['MODULES'] = 'Modules';
$text['SETTINGS'] = 'Settings';
global $lang;
if($lang == 'de')
	$text['LANGUAGES'] = 'Spräche';
else if($lang == 'fr')
{
	$text['LANGUAGES'] = 'Langages';
	$text['MODULE_NAME'] = 'Nom du module';
	$text['PORTAL_ADMINISTRATION'] = 'Administration du portail';
}
_lang($text);


function admin_admin($args)
{
	global $user_id, $debug;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title admin">'._html_safe(PORTAL_ADMINISTRATION)
			."</h1>\n");
	print('<h2><img src="modules/admin/icon.png" alt=""/> '
			._html_safe(SETTINGS).'</h2>'."\n");
	include('./modules/admin/settings.tpl');
	print('<h2><img src="modules/admin/lang.png" alt=""/> '
			._html_safe(LANGUAGES).'</h2>'."\n");
	if(($lang = _sql_array('SELECT lang_id AS apply_id, name, enabled'
			.' FROM daportal_lang')) == FALSE)
		_error('Unable to list languages');
	else
	{
		for($cnt = count($lang), $i = 0; $i < $cnt; $i++)
		{
			$lang[$i]['name'] = _html_safe($lang[$i]['name']);
			$lang[$i]['apply_module'] = 'admin';
			$lang[$i]['apply_id']
				= _html_safe($lang[$i]['apply_id']);
			$lang[$i]['enabled'] = ($lang[$i]['enabled']
					== SQL_TRUE) ? 'enabled' : 'disabled';
			$lang[$i]['enabled'] = '<img src="icons/16x16/'
				.$lang[$i]['enabled'].'.png" alt="'
				.$lang[$i]['enabled'].'" title="'
				.($lang[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		}
		$toolbar = array();
		$toolbar[] = array('title' => DISABLE,
				'icon' => 'icons/16x16/disabled.png',
				'action' => 'lang_disable');
		$toolbar[] = array('title' => ENABLE,
				'icon' => 'icons/16x16/enabled.png',
				'action' => 'lang_enable');
		_module('explorer', 'browse_trusted', array('entries' => $lang,
				'class' => array('enabled' => ENABLED),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'admin', 'action' => 'admin'));
	}
	print('<h2><img src="modules/admin/icon.png" alt=""/> '
			._html_safe(MODULES).'</h2>'."\n");
	if(($modules = _sql_array('SELECT module_id AS apply_id, name AS module'
			.', enabled FROM daportal_module'
			.' ORDER BY module ASC')) == FALSE)
		return _error('Could not list modules');
	for($cnt = count($modules), $i = 0; $i < $cnt; $i++)
	{
		$module = $modules[$i]['module'];
		$modules[$i]['icon'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['thumbnail'] = 'modules/'.$module.'/icon.png';
		$modules[$i]['action'] = 'admin';
		$modules[$i]['apply_module'] = 'admin';
		$modules[$i]['enabled'] = ($modules[$i]['enabled'] == SQL_TRUE)
				? 'enabled' : 'disabled';
		$modules[$i]['enabled'] = '<img src="icons/16x16/'
			.$modules[$i]['enabled'].'.png" alt="'
			.$modules[$i]['enabled'].'" title="'
			.($modules[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
		$modules[$i]['module_name'] = '<a href="index.php?module='
			._html_safe_link($module).'">'
			._html_safe($module).'</a>';
		$title = '';
		include('./modules/'.$module.'/desktop.php');
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
	_module('explorer', 'browse_trusted', array('entries' => $modules,
			'class' => array('enabled' => ENABLED,
					'module_name' => MODULE_NAME),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'admin', 'action' => 'admin'));
}


function admin_default()
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$res = _sql_array('SELECT name FROM daportal_module ORDER BY name ASC');
	$modules = array();
	foreach($res as $r)
	{
		if(($m = _module_desktop($r['name'])) == FALSE
				|| $m['admin'] != 1)
			continue;
		$modules[] = $m;
	}
	include('./modules/admin/default.tpl');
}


function admin_lang_disable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_lang SET enabled='0'"
			." WHERE lang_id='".$args['id']."'") == FALSE)
		_error('Unable to update language');
}


function admin_lang_enable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_lang SET enabled='1'"
			." WHERE lang_id='".$args['id']."'") == FALSE)
		_error('Unable to update language');
}


function admin_module_disable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='0'"
			." WHERE module_id='".$args['id']."'") == FALSE)
		_error('Unable to update module');
}


function admin_module_enable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='1'"
			." WHERE module_id='".$args['id']."'") == FALSE)
		_error('Unable to update module');
}


function admin_settings_update($args)
{
	global $user_id, $debug;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$debug = 1;
	if(isset($args['debug']) && $args['debug'] == 'on')
		$_SESSION['debug'] = 1;
	else
		unset($_SESSION['debug']);
	header('Location: index.php?module=admin&action=admin');
}


function admin_system($args)
{
	global $title, $html;

	$title.=' - Administration';
	if($_SERVER['REQUEST_METHOD'] == 'POST'
			&& $args['action'] == 'settings_update')
		$html = 0;
}

?>
