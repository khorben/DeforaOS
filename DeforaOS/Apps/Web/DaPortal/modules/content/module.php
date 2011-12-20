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
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['CONTENT_ADMINISTRATION'] = 'Content administration';
$text['CONTENT_UPDATE'] = 'Content update';
$text['CONTENTS'] = 'Content';
global $lang;
if($lang == 'fr')
{
	$text['CONTENT_ADMINISTRATION'] = 'Gestion des contenus';
	$text['CONTENT_UPDATE'] = "Mise à jour d'un contenu";
	$text['CONTENTS'] = 'Contenus';
}
_lang($text);


//ContentModule
class ContentModule extends Module
{
	//public
	//methods
	//useful
	//ContentModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'insert':
			case 'system':
			case 'update':
				return $this->$action($args);
			default:
				return $this->_default($args);
		}
		return FALSE;
	}


//ContentModule::admin
protected function admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return content_update(array('id' => $args['id']));
	print('<h1 class="title content">'._html_safe(CONTENT_ADMINISTRATION)
			."</h1>\n");
	$sql = 'SELECT content_id AS id, timestamp AS date, name AS module'
		.', daportal_user.user_id AS user_id, username, title AS name'
		.', daportal_content.enabled AS enabled FROM daportal_content'
		.', daportal_module, daportal_user WHERE'
		.' daportal_content.module_id=daportal_module.module_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' ORDER BY timestamp DESC';
	$contents = _sql_array($sql);
	if(!is_array($contents))
		return _error('Could not list contents');
	$cnt = count($contents);
	for($i = 0; $i < $cnt; $i++)
	{
		$contents[$i]['icon'] = '';
		if(($d = _module_desktop($contents[$i]['module'])) != FALSE)
			$contents[$i]['icon'] = $d['icon'];
		$contents[$i]['thumbnail'] = 'icons/48x48/'
			.$contents[$i]['icon'];
		$contents[$i]['icon'] = 'icons/16x16/'.$contents[$i]['icon'];
		$contents[$i]['name'] = _html_safe($contents[$i]['name']);
		$contents[$i]['username'] = '<a href="'._html_link('user',
			FALSE, $contents[$i]['user_id']).'">'
				._html_safe($contents[$i]['username']).'</a>';
		$contents[$i]['module'] = 'content';
		$contents[$i]['apply_module'] = 'content';
		$contents[$i]['action'] = 'update';
		$contents[$i]['apply_id'] = $contents[$i]['id'];
		$contents[$i]['enabled'] = ($contents[$i]['enabled']
				== SQL_TRUE) ? 'enabled' : 'disabled';
		$contents[$i]['enabled'] = '<img src="icons/16x16/'
				.$contents[$i]['enabled'].'.png" alt="'
				.$contents[$i]['enabled'].'" title="'
				.($contents[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
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
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => _module_link('content', 'admin'),
			'onclick' => 'location.reload(); return false');
	_module('explorer', 'browse_trusted', array('entries' => $contents,
			'class' => array('enabled' => ENABLED,
				'username' => AUTHOR, 'date' => DATE),
			'view' => 'details', 'toolbar' => $toolbar,
			'module' => 'content', 'action' => 'admin'));
}


	//ContentModule::_default
	protected function _default($args)
	{
		global $user_id;

		if(!isset($args['id']))
		{
			return include('./modules/content/default.tpl');
		}
		require_once('./system/user.php');
		if(_user_admin($user_id))
			$where = '';
		else
			$where = " AND daportal_content.enabled='1'";
		$module = _sql_single('SELECT name FROM daportal_content'
				.', daportal_module WHERE daportal_content'
				.'.module_id=daportal_module.module_id'
				.$where." AND content_id='".$args['id']."'");
		if($module == FALSE)
			return _error('Could not display content');
		_module($module, FALSE, array('id' => $args['id']));
	}


//ContentModule::delete
protected function delete($args)
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


//ContentModule::system
protected function system($args)
{
	global $title, $error;

	$title.=' - '.CONTENTS;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	if($args['action'] == 'update')
		$error = $this->_content_system_update($args);
}

private function _content_system_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	if(_sql_query('UPDATE daportal_content SET '
			." title='".$args['title']."'"
			.", timestamp='".$args['timestamp']."'"
			.", user_id='".$args['user_id']."'"
			.", content='".$args['content']."'"
			.", enabled='".$args['enabled']."'"
			." WHERE content_id='".$args['id']."'") == FALSE)
		return 'Unable to update content';
	header('Location: '._module_link('content', 'admin'));
}


//ContentModule::update
protected function update($args)
{
	global $error, $user_id;

	if(isset($error) && strlen($error))
		_error($error);
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	if(!is_numeric($id))
		return _error('Invalid content');
	$content = _sql_array('SELECT timestamp, user_id, title, content'
			.', enabled FROM daportal_content'
			." WHERE content_id='$id'");
	if(!is_array($content) || count($content) != 1)
		return _error(INVALID_ARGUMENT);
	$content = $content[0];
	$users = _sql_array('SELECT user_id, username FROM daportal_user'
			.' ORDER BY username ASC');
	$title = CONTENT_UPDATE;
	include('./modules/content/update.tpl');
}
}

?>
