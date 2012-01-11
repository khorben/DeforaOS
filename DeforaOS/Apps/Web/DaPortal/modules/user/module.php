<?php //$Id$
//Copyright (c) 2004-2012 Pierre Pronchery <khorben@defora.org>
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
$text['ALREADY_LOGGED_IN'] = 'Already logged in';
$text['APPEARANCE'] = 'Appearance';
$text['CONFIRMATION_FAILED'] = 'Confirmation failed';
$text['CREATE'] = 'Create';
$text['DEFAULT_THEME'] = 'Default theme';
$text['DEFAULT_VIEW'] = 'Default view';
$text['EMAIL'] = 'e-mail';
$text['EMAIL_ALREADY_ASSIGNED'] = 'e-mail already assigned';
$text['EMAIL_INVALID'] = 'e-mail is not valid';
$text['FULLNAME'] = 'Full name';
$text['MY_CONTENT'] = 'My content';
$text['MY_PROFILE'] = 'My profile';
$text['NONE'] = 'None';
$text['NEW_USER'] = 'New user';
$text['REGISTER'] = 'Register';
$text['S_CONTENT'] = "'s content";
$text['S_PAGE'] = "'s page";
$text['SETTINGS'] = 'Settings';
$text['USER_ALREADY_ASSIGNED'] = 'Username already assigned';
$text['USER_LOGIN'] = 'User login';
$text['USER_MODIFICATION'] = 'User modification';
$text['USER_REGISTRATION'] = 'User registration';
$text['USERS'] = 'Users';
$text['USERS_ADMINISTRATION'] = 'Users administration';
$text['VIEW_DETAILS'] = 'Details';
$text['VIEW_LIST'] = 'List';
$text['VIEW_THUMBNAILS'] = 'Thumbnails';
$text['WRONG_PASSWORD'] = 'Wrong password';
$text['YOUR_PASSWORD_IS'] = 'Your password is';
global $lang;
if($lang == 'de')
{
	include('./modules/user/lang.de.php');
}
else if($lang == 'fr')
{
	include('./modules/user/lang.fr.php');
}
_lang($text);


//UserModule
class UserModule extends Module
{
	//public
	//methods
	//useful
	//UserModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'appearance':
			case 'config_update':
			case 'confirm':
			case 'delete':
			case 'disable':
			case 'display':
			case 'enable':
			case 'login':
			case 'logout':
			case 'modify':
			case 'register':
			case 'system':
			case 'update':
				return $this->$action($args);
			case 'new':
				return $this->_new($args);
			default:
				return $this->_default($args);
		}
		return FALSE;
	}


	private function _modify($args)
	{
		global $user_id;

		if($user_id == 0)
			return _error(PERMISSION_DENIED);
		$id = $user_id;
		require_once('./system/user.php');
		$admin = _user_admin($id) ? TRUE : FALSE;
		if(isset($args['id']) && $admin == TRUE)
			$id = $args['id'];
		$user = _sql_array('SELECT user_id, username, enabled, admin'
				.', fullname, email'
				." FROM daportal_user WHERE user_id='$id'");
		if(!is_array($user) || count($user) != 1)
			return _error('Invalid user');
		$user = $user[0];
		include('./modules/user/user_update.tpl');
	}


	private function _password_mail($id, $username, $email,
			$password = FALSE)
		//FIXME weak passwords and keys...?
	{
		if($password == FALSE)
			$password = $this->_password_new();
		$key = md5($this->_password_new());
		if(_sql_query('INSERT INTO daportal_user_register'
				.' (user_id, key)'
				." VALUES ('$id', '$key')") == FALSE)
			return _error('Could not create confirmation key');
		$message = YOUR_PASSWORD_IS." '$password'\n\n"
				."Please click on the following link to"
				." confirm:\n"
				._module_link_full('user', 'confirm', FALSE,
						FALSE, 'key='.$key);
		require_once('./system/mail.php');
		_mail('Administration Team', $username.' <'.$email.'>',
			'User confirmation', $message);
	}


	private function _password_new()
	{
		$string = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
				.'0123456789';
		$password = '';
		for($i = 0; $i < 8; $i++)
			$password.=$string[rand(0, strlen($string)-1)];
		return $password;
	}


//UserModule::admin
protected function admin($args)
{
	global $user_id;

	if(isset($args['id']))
		return $this->modify($args);
	require_once('./system/user.php');
	require_once('./system/icon.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title users">'._html_safe(USERS_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('user')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'user';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title users">'._html_safe(USERS).'</h2>'."\n");
	$order = 'name ASC';
	if(isset($args['sort']))
		switch($args['sort'])
		{
			case 'admin':	$order = 'admin DESC';	break;
			case 'fullname':$order = 'fullname ASC';break;
			case 'email':	$order = 'email ASC';	break;
			case 'enabled':	$order = 'enabled DESC';break;
		}
	$users = _sql_array('SELECT user_id AS id, username AS name, enabled'
			.', admin, fullname, email FROM daportal_user'
			.' ORDER BY '.$order);
	if(!is_array($users))
		return _error('Unable to list users');
	$count = count($users);
	for($i = 0; $i < $count; $i++)
	{
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'admin';
		$users[$i]['icon'] = _icon('user', 16);
		$users[$i]['thumbnail'] = _icon('user', 48);
		$users[$i]['name'] = _html_safe($users[$i]['name']);
		$users[$i]['apply_module'] = 'user';
		$users[$i]['apply_id'] = $users[$i]['id'];
		$users[$i]['enabled'] = $users[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$users[$i]['enabled'] = '<img src="icons/16x16/'
				.$users[$i]['enabled'].'.png" alt="'
				.$users[$i]['enabled'].'" title="'
				.($users[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$users[$i]['admin'] = $users[$i]['admin'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$users[$i]['admin'] = '<img src="icons/16x16/'
				.$users[$i]['admin'].'.png" alt="'
				.$users[$i]['admin'].'" title="'
				.($users[$i]['admin'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$users[$i]['fullname'] = _html_safe($users[$i]['fullname']);
		$users[$i]['email'] = '<a href="mailto:'.$users[$i]['email']
				.'">'._html_safe($users[$i]['email']).'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_USER, 'class' => 'new',
			'link' => _module_link('user', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $users,
				'toolbar' => $toolbar, 'view' => 'details',
				'class' => array('enabled' => ENABLED,
					'admin' => 'Admin',
					'fullname' => 'Full name',
					'email' => EMAIL),
				'module' => 'user', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
				: 'name'));
}


	protected function appearance($args)
	{
		global $theme;

		print('<h1 class="title appearance">'._html_safe(APPEARANCE)
				."</h1>\n");
		$themes = array();
		$filename = dirname($_SERVER['SCRIPT_FILENAME']).'/themes';
		if(($dir = opendir($filename))!= FALSE)
			while(($de = readdir($dir)) != FALSE)
			{
				if(($len = strlen($de)) < 5
						|| substr($de, -4) != '.css')
					continue;
				$themes[] = substr($de, 0, -4);
			}
		usort($themes, 'strcmp');
		$views = array('details' => VIEW_DETAILS, 'list' => VIEW_LIST,
				'thumbnails' => VIEW_THUMBNAILS);
		if(isset($_SESSION['view']))
			$view = $_SESSION['view'];
		//FIXME go to the users' page
		include('./modules/user/appearance.tpl');
	}


	protected function config_update($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return user_admin(array());
	}


	protected function confirm($args)
	{
		global $error, $info;

		include('./modules/user/user_confirm.tpl');
	}


	protected function _default($args)
	{
		global $user_id, $user_name;

		if(isset($args['id']))
			return $this->display($args);
		if($user_id == 0)
			return $this->login($args);
		if(($user = _sql_array('SELECT user_id, username, admin'
				.' FROM daportal_user'
				." WHERE user_id='$user_id'")) == FALSE
				|| count($user) != 1)
			return _error('Invalid user');
		$user = $user[0];
		print('<h1 class="title home">'._html_safe($user['username']
					.S_PAGE)."</h1>\n");
		$modules = array();
		require_once('./system/user.php');
		require_once('./system/icon.php');
		if(_user_admin($user_id) && ($d = _module_desktop('admin'))
				!= FALSE)
			$modules[] = array('name' => $d['title'],
					'icon' => _icon('admin', 16),
					'thumbnail' => _icon('admin', 48),
					'module' => 'admin',
					'action' => 'default');
		$modules[] = array('name' => APPEARANCE,
				'icon' => _icon('appearance', 16),
				'thumbnail' => _icon('appearance', 48),
				'module' => 'user', 'action' => 'appearance');
		$modules[] = array('name' => MY_CONTENT,
				'icon' => _icon('content', 16),
				'thumbnail' => _icon('content', 48),
				'module' => 'user', 'action' => 'default',
				'id' => $user_id);
		$modules[] = array('name' => MY_PROFILE,
				'icon' => _icon('admin', 16),
				'thumbnail' => _icon('admin', 48),
				'module' => 'user', 'action' => 'modify',
				'id' => $user_id);
		$modules[] = array('name' => LOGOUT,
				'icon' => _icon('logout', 16),
				'thumbnail' => _icon('logout', 48),
				'module' => 'user', 'action' => 'logout');
		_module('explorer', 'browse', array('entries' => $modules,
					'view' => 'thumbnails',
					'toolbar' => 0));
	}


	//UserModule::delete
	protected function delete($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id)
				|| $_SERVER['REQUEST_METHOD'] != 'POST')
			return _error(PERMISSION_DENIED);
		_sql_query('DELETE FROM daportal_user_register'
				." WHERE user_id='".$args['id']."'");
		if(!_sql_query('DELETE FROM daportal_user'
				." WHERE user_id='".$args['id']."'"))
			return _error('Could not delete user');
	}


	//UserModule::disable
	protected function disable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id)
				|| $_SERVER['REQUEST_METHOD'] != 'POST')
			return _error(PERMISSION_DENIED);
		if(!_sql_query('UPDATE daportal_user SET enabled='."'0'"
				." WHERE user_id='".$args['id']."'"))
			return _error('Could not disable user');
	}


//UserModule::display
protected function display($args)
{
	global $user_id;

	require_once('./system/icon.php');
	if(!isset($args['id']))
		$args['id'] = $user_id;
	else if(!is_numeric($args['id']))
		return _error('Invalid user ID');
	$user = _sql_array('SELECT user_id AS id, username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['id']."'");
	if(!is_array($user) || count($user) != 1)
		return _error('Invalid user');
	$user = $user[0];
	$res = _sql_array("SELECT name FROM daportal_module WHERE enabled='1'"
			.' ORDER BY name ASC');
	if(!is_array($res))
		return _error('Could not list modules');
	print('<h1 class="title user">'._html_safe($user['username'].S_CONTENT)
			."</h1>\n");
	$entries = array();
	foreach($res as $r)
	{
		if(($d = _module_desktop($r['name'])) == FALSE
				|| !is_array($d['user']))
			continue;
		foreach($d['user'] as $e)
		{
			if(!isset($e['module']))
				$e['module'] = $r['name'];
			if(!isset($e['action']))
				$e['action'] = 'default';
			$e['icon'] = isset($e['icon']) ? $e['icon']
				: $d['icon'];
			$e['thumbnail'] = _icon($e['icon'], 48);
			$e['icon'] = _icon($e['icon'], 16);
			$e['args'] = 'user_id='.$user['id'];
			$entries[] = $e;
		}
	}
	_module('explorer', 'browse', array('entries' => $entries,
				'view' => 'thumbnails', 'toolbar' => 0));
}


	//UserModule::enable
	protected function enable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id)
				|| $_SERVER['REQUEST_METHOD'] != 'POST')
			return _error(PERMISSION_DENIED);
		if(!_sql_query('UPDATE daportal_user SET enabled='."'1'"
				." WHERE user_id='".$args['id']."'"))
			return _error('Could not enable user');
	}


	//UserModule::insert
	protected function insert($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(preg_match('/^[a-z]{1,9}$/', $args['username']) != 1)
			return _error('Username must be lower-case and no'
					.' longer than 9 characters', 1);
		if(strlen($args['password1']) < 1
				|| $args['password1'] != $args['password2'])
			return _error('Passwords must be non-empty and match',
					1);
		$password = md5($args['password1']);
		if(!_sql_query('INSERT INTO daportal_user (username, password'
				.', enabled, admin, email) VALUES ('
				."'".$args['username']."', '$password'"
				.", '".(isset($args['enabled']) ? '1' : '0')."'"
				.", '".(isset($args['admin']) ? '1' : '0')."'"
				.", '".$args['email']."');"))
			return _error('Could not insert user');
		$id = _sql_id('daportal_user', 'user_id');
		user_display(array('id' => $id));
	}


	//UserModule::login
	protected function login($args)
	{
		global $user_id, $error;

		if($user_id != 0)
			return $this->_default($args);
		$register = _config_get('user', 'register') ? 1 : 0;
		$username = isset($args['username'])
			? stripslashes($args['username']) : '';
		include('./modules/user/user_login.tpl');
		if(isset($error) && strlen($error))
			_error($error);
	}


	//UserModule::logout
	protected function logout($args)
	{
		global $user_id;

		if($user_id != 0)
			return _error('Unable to logout');
		return include('./modules/user/user_logout.tpl');
	}


	//UserModule::modify
	protected function modify($args)
	{
		print('<h1 class="user title">'.USER_MODIFICATION."</h1>\n");
		return $this->_modify($args);
	}


	//UserModule::new
	protected function _new($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="user title">'.NEW_USER."</h1>\n");
		$admin = 1;
		include('./modules/user/user_update.tpl');
	}


//UserModule::register
protected function register($args)
{
	global $user_id;

	if(_config_get('user', 'register') != TRUE)
		return _error(PERMISSION_DENIED);
	if($user_id)
		return _error(ALREADY_LOGGED_IN);
	$message = '';
	if($_SERVER['REQUEST_METHOD'] == 'POST' && isset($args['username'])
			&& isset($args['email']))
	{
		if(preg_match('/^[a-z]{1,9}$/', $args['username']) != 1)
			$message = 'Username must be lower-case and no longer'
				.' than 9 characters';
		else
		{
			if(_sql_single('SELECT username FROM daportal_user'
					." WHERE username='".$args['username']
					."'") == FALSE)
			{
				$email = $args['email'];
				if(preg_match('/^[a-z0-9_.-]+@[a-z0-9_.-]+\.'
							.'[a-z]{1,4}$/i',
							$email) != 1)
					$message = EMAIL_INVALID;
				else if(_sql_array('SELECT email'
						.' FROM daportal_user'
						." WHERE email='$email'")
						== FALSE)
					return $this->_register_mail(
							$args['username'],
							$email);
				else
					$message = EMAIL_ALREADY_ASSIGNED;
			}
			else
				$message = USER_ALREADY_ASSIGNED;
		}
	}
	print('<h1 class="title user">'._html_safe(USER_REGISTRATION)
			."</h1>\n");
	if(strlen($message))
		_error($message);
	include('./modules/user/user_register.tpl');
}

	private function _register_mail($username, $email)
	{
		$password = $this->_password_new();
		_info('New password is: '.$password);
		if(_sql_query('INSERT INTO daportal_user (username, password'
				.', enabled, admin, email) VALUES ('
				."'$username', '".md5($password)."', '0', '0'"
				.", '$email')") == FALSE)
			return _error('Could not insert user');
		$id = _sql_id('daportal_user', 'user_id');
		include('./modules/user/user_pending.tpl');
		$this->_password_mail($id, $username, $email, $password);
	}


	//UserModule::system
	protected function system($args)
	{
		global $html, $error;

		if(!isset($args['action']))
			return;
		if($_SERVER['REQUEST_METHOD'] == 'POST')
		{
			if($args['action'] == 'login')
				$error = $this->_system_login($args);
			else if($args['action'] == 'config_update')
				$error = $this->_system_config_update($args);
			else if($args['action'] == 'error')
				$args['action'] = 'default';
			else if($args['action'] == 'appearance')
				return $this->_system_appearance($args);
			else if($args['action'] == 'update')
				$error = $this->_system_update($args);
		}
		else if($_SERVER['REQUEST_METHOD'] == 'GET')
		{
			if($args['action'] == 'logout')
				$this->_system_logout();
			else if($args['action'] == 'confirm'
					&& isset($args['key']))
				$this->_system_confirm($args['key']);
			else if($args['action'] == 'error')
				$args['action'] = 'default';
		}
	}

	private function _system_appearance($args)
	{
		global $debug, $user_id;

		unset($_SESSION['theme']);
		$filename = dirname($_SERVER['SCRIPT_FILENAME']).'/themes/'
			.$args['theme'].'.css';
		if(isset($args['theme'])
				&& strpos($args['theme'], '/') === FALSE
				&& is_readable($filename))
			$_SESSION['theme'] = $args['theme'];
		unset($_SESSION['view']);
		$views = array('details', 'list', 'thumbnails');
		if(isset($args['view']) && in_array($args['view'], $views))
			$_SESSION['view'] = $args['view'];
		require_once('./system/user.php');
		if(_user_admin($user_id))
		{
			unset($_SESSION['debug']);
			if(isset($args['debug']))
				$_SESSION['debug'] = 1;
		}
		header('Location: '._module_link('user', 'appearance'));
		exit(0);
	}

	private function _system_config_update($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		$args['user_register'] = isset($args['user_register']) ? TRUE
			: FALSE;
		$args['user_manual'] = isset($args['user_manual']) ? TRUE
			: FALSE;
		_config_update('user', $args);
		header('Location: '._module_link('user', 'admin'));
		exit(0);
	}

	private function _system_confirm($key)
	{
		global $error;

		$error = CONFIRMATION_FAILED;
		//FIXME remove expired registration keys
		//FIXME use a transaction
		$user = _sql_array('SELECT daportal_user.user_id AS user_id'
			.', daportal_user.username AS username'
			.' FROM daportal_user, daportal_user_register'
			.' WHERE daportal_user.user_id'
			.'=daportal_user_register.user_id AND key='."'$key'");
		if(!is_array($user) || count($user) != 1)
			return;
		$user = $user[0];
		if(_config_get('user', 'manual') == FALSE)
			return $this->_system_confirm_auto($key, $user);
		return $this->_system_confirm_manual($key, $user);
	}

	private function _system_confirm_auto($key, $user)
	{
		if(_sql_query('UPDATE daportal_user SET enabled='."'1'"
				." WHERE user_id='".$user['user_id']."'")
				== FALSE)
			return _error('Could not enable user');
		if(_sql_query("DELETE FROM daportal_user_register"
					." WHERE key='$key'") == FALSE)
			_error('Could not remove registration key');
		if(strlen(session_id()) == 0)
			session_start();
		$_SESSION['user_id'] = $user['user_id'];
		$_SESSION['user_name'] = $user['username'];
		header('Location: '._module_link('user'));
		exit(0);
	}

	private function _system_confirm_manual($key, $user)
	{
		global $info;

		if(_sql_query("DELETE FROM daportal_user_register"
					." WHERE key='$key'") == FALSE)
			_error('Could not remove registration key');
		if(_sql_query("DELETE FROM daportal_user_register"
					." WHERE key='$key'") == FALSE)
			_error('Could not remove registration key');
		$info = 'Your confirmation was acknowledged by the system.'
			.' Your account request will be manually enabled'
			.' shortly. Thanks!';
		//send mail
		$admins = _sql_array('SELECT username, email FROM daportal_user'
				." WHERE enabled='1' AND admin='1'");
		if(!is_array($admins))
			return _error('Could not list moderators', 0);
		$to = '';
		$comma = '';
		foreach($admins as $a)
		{
			$to.=$comma.$a['username'].' <'.$a['email'].'>';
			$comma = ', ';
		}
		$subject = 'User registration: '.$user['username'];
		$content = "A new user is awaiting moderation at:\n"
			.'https://'.$_SERVER['SERVER_NAME']
			._module_link('user', 'admin', $user['user_id'])."\n";
		require_once('./system/mail.php');
		_mail('Administration Team', $to, $subject, $content);
	}

	private function _system_login($args)
	{
		global $user_id; 

		if(strlen(session_id()) == 0)
			session_start();
		if(!isset($args['username']) || !isset($args['password']))
			return INVALID_ARGUMENT;
		$password = md5(stripslashes($args['password']));
		$query = 'SELECT user_id, username FROM daportal_user'
			." WHERE username='".$args['username']."'"
			." AND password='$password' AND enabled='1'";
		$res = _sql_array($query);
		if(!is_array($res) || count($res) != 1)
			return WRONG_PASSWORD;
		$res = $res[0];
		$_SESSION['user_id'] = $res['user_id'];
		$_SESSION['user_name'] = $res['username'];
		header('Location: '._module_link('user'));
		exit(0);
	}

	private function _system_logout()
	{
		global $user_id, $user_name;

		$_SESSION['user_id'] = 0;
		$_SESSION['user_name'] = 'Anonymous';
		unset($_SESSION['debug']);
		unset($_SESSION['theme']);
		$user_id = $_SESSION['user_id'];
		$user_name = $_SESSION['user_name'];
	}

	private function _system_update($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(_user_admin($user_id))
			$id = $args['id'];
		else if($user_id != 0)
			$id = $user_id;
		else
			return PERMISSION_DENIED;
		if($args['password1'] != $args['password2'])
			return 'Passwords do not match';
		$password = '';
		if(strlen($args['password1']))
			$password = "password='".md5($args['password1'])."'";
		if(_user_admin($user_id))
			$sql = 'UPDATE daportal_user SET username='
				."'".$args['username']."', enabled='"
				.(isset($args['enabled'])
						&& $args['enabled'] == 'on'
						? '1' : '0')."', admin='"
				.(isset($args['admin'])
						&& $args['admin'] == 'on'
						? '1' : '0')."'"
				.", fullname='".$args['fullname']."'"
				.", email='".$args['email']."'"
				.(strlen($password) ? ', '.$password
						: '')." WHERE user_id='$id'";
		else if(strlen($password))
			$sql = 'UPDATE daportal_user SET '.$password
				." WHERE user_id='$id'";
		print_r($sql);
		if(strlen($sql) && _sql_query($sql) === FALSE)
			return 'Could not update user';
		header('Location: '._module_link_full('user', 'display', $id));
		exit(0);
	}


	//UserModule::update
	protected function update($args)
	{
		global $error;

		print('<h1 class="user title">'.USER_MODIFICATION."</h1>\n");
		if(isset($error) && strlen($error))
			_error($error);
		$this->_modify($args);
	}
}

?>
