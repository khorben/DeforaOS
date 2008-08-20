<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
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
{
	$text['LANGUAGES'] = 'Spräche';
	$text['MODULE_NAME'] = 'Module Name';
}
else if($lang == 'fr')
{
	$text['LANGUAGES'] = 'Langages';
	$text['MODULE_NAME'] = 'Nom du module';
	$text['PORTAL_ADMINISTRATION'] = 'Administration du portail';
	$text['SETTINGS'] = 'Paramètres';
}
_lang($text);


function admin_admin($args)
{
	global $user_id, $debug;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title admin">'._html_safe(PORTAL_ADMINISTRATION)
			."</h1>\n".'<h2 class="title settings">'
			._html_safe(SETTINGS)."</h2>\n");
	if(($configs = _config_list('admin')))
	{
		$module = 'admin';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title language">'._html_safe(LANGUAGES).'</h2>'."\n");
	if(($lang = _sql_array('SELECT lang_id AS apply_id, name, enabled'
			.' FROM daportal_lang ORDER BY name ASC')) == FALSE)
		_error('Unable to list languages');
	else
	{
		for($cnt = count($lang), $i = 0; $i < $cnt; $i++)
		{
			$lang[$i]['icon'] = 'icons/16x16/language.png';
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
		$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
				'action' => 'lang_disable');
		$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
				'action' => 'lang_enable');
		_module('explorer', 'browse_trusted', array('entries' => $lang,
				'class' => array('enabled' => ENABLED),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'admin', 'action' => 'admin'));
	}
	print('<h2 class="title modules">'._html_safe(MODULES).'</h2>'."\n");
	if(($modules = _sql_array('SELECT module_id AS apply_id, name AS module'
			.', enabled FROM daportal_module'
			.' ORDER BY module ASC')) == FALSE)
		return _error('Could not list modules');
	for($cnt = count($modules), $i = 0; $i < $cnt; $i++)
	{
		$module = $modules[$i]['module'];
		$modules[$i]['icon'] = '';
		$modules[$i]['apply_module'] = 'admin';
		$modules[$i]['enabled'] = ($modules[$i]['enabled'] == SQL_TRUE)
				? 'enabled' : 'disabled';
		$modules[$i]['enabled'] = '<img src="icons/16x16/'
			.$modules[$i]['enabled'].'.png" alt="'
			.$modules[$i]['enabled'].'" title="'
			.($modules[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
		$modules[$i]['module_name'] = _html_safe($module);
		if(($d = _module_desktop($module)) != FALSE
				&& strlen($d['title']))
		{
			$modules[$i]['name'] = $d['title'];
			$modules[$i]['icon'] = $d['icon'];
			if($d['admin'] == 1)
			{
				$modules[$i]['action'] = 'admin';
				$modules[$i]['module_name'] =
					'<a href="'._html_link($module).'">'
					._html_safe($module).'</a>';
			}
		}
		else
			$modules[$i]['name'] = $modules[$i]['module'];
		$modules[$i]['thumbnail'] = 'icons/48x48/'.$modules[$i]['icon'];
		$modules[$i]['icon'] = 'icons/16x16/'.$modules[$i]['icon'];
		$modules[$i]['name'] = _html_safe($modules[$i]['name']);
	}
	$toolbar = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'module_disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'module_enable');
	_module('explorer', 'browse_trusted', array('entries' => $modules,
			'class' => array('enabled' => ENABLED,
					'module_name' => MODULE_NAME),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'admin', 'action' => 'admin'));
}


function admin_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return admin_admin(array());
}


function admin_default()
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$res = _sql_array('SELECT name FROM daportal_module'
			." WHERE enabled='1' ORDER BY name ASC");
	$modules = array();
	foreach($res as $r)
	{
		if(($m = _module_desktop($r['name'])) == FALSE
				|| $m['admin'] != 1)
			continue;
		$m['thumbnail'] = 'icons/48x48/'.$m['icon'];
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


function admin_system($args)
{
	global $title, $error;

	$title.=' - '.ADMINISTRATION;
	if(!isset($args['action'])
			|| $_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'config_update':
			$error = _system_config_update($args);
			break;
	}
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('admin', $args);
	header('Location: '._module_link('admin', 'admin'));
	exit(0);
}

?>
