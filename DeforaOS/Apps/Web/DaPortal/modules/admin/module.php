<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


require_once('./system/user.php');


//lang
$text = array();
$text['ADMINISTRATION'] = 'Administration';
$text['GLOBAL_CONFIGURATION'] = 'Global configuration';
$text['INSERT'] = 'Insert';
$text['INSERT_MODULE'] = 'Insert module';
$text['LANGUAGES'] = 'Languages';
$text['PORTAL_ADMINISTRATION'] = 'Portal administration';
$text['MODULE_NAME'] = 'Module name';
$text['MODULES'] = 'Modules';
$text['NEW_MODULE'] = 'New module';
$text['SETTINGS'] = 'Settings';
global $lang;
if($lang == 'de')
{
	$text['LANGUAGES'] = 'Spräche';
	$text['MODULE_NAME'] = 'Module Name';
}
else if($lang == 'fr')
{
	$text['INSERT'] = 'Ajouter';
	$text['INSERT_MODULE'] = 'Ajouter un module';
	$text['LANGUAGES'] = 'Langages';
	$text['MODULE_NAME'] = 'Nom du module';
	$text['PORTAL_ADMINISTRATION'] = 'Administration du portail';
	$text['SETTINGS'] = 'Paramètres';
}
_lang($text);


//AdminModule
class AdminModule extends Module
{
	//public
	//methods
	//useful
	//AdminModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch($request->getAction())
		{
			case 'admin':
				return $this->admin($args);
			case 'config_update':
				return $this->config_update($args);
			case 'lang_disable':
				return $this->lang_disable($args);
			case 'lang_enable':
				return $this->lang_enable($args);
			case 'module_delete':
				return $this->module_delete($args);
			case 'module_disable':
				return $this->module_disable($args);
			case 'module_enable':
				return $this->module_enable($args);
			case 'module_insert':
				return $this->module_insert($args);
			case 'system':
				return $this->system($args);
			default:
				return $this->_default();
		}
		return FALSE;
	}


	//AdminModule::admin
	protected function admin($args)
	{
		require_once('./system/icon.php');
		global $user_id, $debug;

		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title admin">'._html_safe(
					PORTAL_ADMINISTRATION)
				."</h1>\n".'<h2 class="title settings">'
				._html_safe(SETTINGS)."</h2>\n");
		if(($configs = _config_list('admin')))
		{
			$module = 'admin';
			$action = 'config_update';
			include('./system/config.tpl');
		}
		print('<h2 class="title language">'._html_safe(LANGUAGES)
				."</h2>\n");
		$query = 'SELECT lang_id AS apply_id, name, enabled'
			.' FROM daportal_lang ORDER BY name ASC';
		if(($lang = _sql_array($query)) == FALSE)
			_error('Unable to list languages');
		else
		{
			for($cnt = count($lang), $i = 0; $i < $cnt; $i++)
			{
				$lang[$i]['icon'] = _icon('language');
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
			$modules[$i]['thumbnail'] = _icon($modules[$i]['icon'], 48);
			$modules[$i]['icon'] = _icon($modules[$i]['icon'], 16);
			$modules[$i]['name'] = _html_safe($modules[$i]['name']);
		}
		$toolbar = array();
		$toolbar[] = array('title' => NEW_MODULE, 'class' => 'new',
				'link' => _module_link('admin', 'module_insert'));
		$toolbar[] = array();
		$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
				'action' => 'module_disable');
		$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
				'action' => 'module_enable');
		$toolbar[] = array();
		$toolbar[] = array('title' => DELETE, 'class' => 'delete',
				'action' => 'module_delete', 'confirm' => 'delete');
		_module('explorer', 'browse_trusted', array('entries' => $modules,
					'class' => array('enabled' => ENABLED,
						'module_name' => MODULE_NAME),
					'toolbar' => $toolbar, 'view' => 'details',
					'module' => 'admin', 'action' => 'admin'));
	}


//AdminModule::config_update
protected function config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return admin_admin(array());
}


//AdminModule::_default
protected function _default()
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/icon.php');
	$res = _sql_array('SELECT name FROM daportal_module'
			." WHERE enabled='1' ORDER BY name ASC");
	$modules = array();
	foreach($res as $r)
	{
		if(($m = _module_desktop($r['name'])) == FALSE
				|| $m['admin'] != 1)
			continue;
		$m['thumbnail'] = _icon($m['icon'], 48);
		$modules[] = $m;
	}
	include('./modules/admin/default.tpl');
}


//AdminModule::lang_disable
protected function lang_disable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_lang SET enabled='0'"
			." WHERE lang_id='".$args['id']."'") == FALSE)
		_error('Unable to update language');
}


//AdminModule::lang_enable
protected function lang_enable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_lang SET enabled='1'"
			." WHERE lang_id='".$args['id']."'") == FALSE)
		_error('Unable to update language');
}


//AdminModule::module_delete
protected function module_delete($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("DELETE FROM daportal_module"
			." WHERE module_id='".$args['id']."'") == FALSE)
		_error('Unable to delete module');
}


//AdminModule::admin_module_disable
protected function module_disable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='0'"
			." WHERE module_id='".$args['id']."'") == FALSE)
		_error('Unable to update module');
}


//AdminModule::module_enable
protected function module_enable($args)
{
	global $user_id;

	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(_sql_query("UPDATE daportal_module SET enabled='1'"
			." WHERE module_id='".$args['id']."'") == FALSE)
		_error('Unable to update module');
}


	//AdminModule::module_insert
	protected function module_insert($args)
	{
		global $error;

		print('<h1 class="title admin">'._html_safe(INSERT_MODULE)
				."</h1>\n");
		if(isset($error) && strlen($error))
			_error($error);
		include('./modules/admin/module_update.tpl');
	}


	//AdminModule::system
	protected function system($args)
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
			case 'module_insert':
				$error = _system_module_insert($args);
				break;
		}
	}

	private function _system_config_update($args)
	{
		global $user_id;

		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		_config_update('admin', $args);
		header('Location: '._module_link('admin', 'admin'));
		exit(0);
	}

	private function _system_module_insert($args)
	{
		global $user_id;

		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		//FIXME add more checks
		if(!isset($args['name']) || strlen($args['name']) == 0)
			return INVALID_ARGUMENT;
		//FIXME perform the same validation test as in system/module.php
		if(!_sql_query('INSERT INTO daportal_module (name, enabled)'
					." VALUES ('".$args['name']."', '0')"))
			return INTERNAL_ERROR;
		header('Location: '._module_link_full('admin', 'admin'));
		exit(0);
	}
}

?>
