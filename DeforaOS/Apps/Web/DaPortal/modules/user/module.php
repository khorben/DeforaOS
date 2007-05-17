<?php //modules/user/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['APPEARANCE'] = 'Appearance';
$text['EMAIL'] = 'e-mail';
$text['EMAIL_ALREADY_ASSIGNED'] = 'e-mail already assigned';
$text['EMAIL_INVALID'] = 'e-mail is not valid';
$text['MY_CONTENT'] = 'My content';
$text['MY_PROFILE'] = 'My profile';
$text['NEW_USER'] = 'New user';
$text['REGISTER'] = 'Register';
$text['USER_ALREADY_ASSIGNED'] = 'Username already assigned';
$text['USER_LOGIN'] = 'User login';
$text['USER_MODIFICATION'] = 'User modification';
$text['USER_REGISTRATION'] = 'User registration';
$text['USERS'] = 'Users';
$text['USERS_ADMINISTRATION'] = 'Users administration';
$text['WRONG_PASSWORD'] = 'Wrong password';
$text['YOUR_PASSWORD_IS'] = 'Your password is ';
global $lang;
if($lang == 'de')
{
	$text['EMAIL_ALREADY_ASSIGNED'] = 'e-mail Adresse schon benutzt';
	$text['MY_CONTENT'] = 'Mein Datei';
	$text['MY_PROFILE'] = 'Mein Konto';
	$text['USER_LOGIN'] = 'Benutzer einloggen';
	$text['WRONG_PASSWORD'] = 'Falsch Passwort';
}
else if($lang == 'fr')
{
	$text['EMAIL_ALREADY_ASSIGNED'] = 'Cet e-mail est déjà utilisé';
	$text['EMAIL_INVALID'] = "Cet e-mail n'est pas valide";
	$text['NEW_USER'] = 'Nouvel utilisateur';
	$text['REGISTER'] = "S'inscrire";
	$text['USER_ALREADY_ASSIGNED'] = 'Cet utilisateur existe déjà';
	$text['USER_LOGIN'] = 'Authentification utilisateur';
	$text['USER_MODIFICATION'] = "Modification d'utilisateur";
	$text['USER_REGISTRATION'] = 'Inscription utilisateur';
	$text['WRONG_PASSWORD'] = 'Mot de passe incorrect';
	$text['YOUR_PASSWORD_IS'] = 'Votre mot de passe est ';
}
_lang($text);


function _password_mail($id, $username, $email, $password = FALSE)
	//FIXME weak passwords and keys...?
{
	if($password == FALSE)
		$password = _password_new();
	$key = md5(_password_new());
	if(_sql_query('INSERT INTO daportal_user_register (user_id, key)'
			.' VALUES ('."'$id', '$key')") == FALSE)
		return _error('Could not create confirmation key');
	$message = YOUR_PASSWORD_IS." '$password'\n\n"
			."Please click on the following link to confirm:\n"
			.(isset($_SERVER['HTTPS']) ? 'https' : 'http')
			.'://'.$_SERVER['SERVER_NAME'].$_SERVER['SCRIPT_NAME']
			.'?module=user&action=confirm&key='.$key;
	require_once('./system/mail.php');
	_mail('Administration Team', $username.' <'.$email.'>',
		'User confirmation', $message);
}


function _password_new()
{
	$string = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
			.'0123456789';
	$password = '';
	for($i = 0; $i < 8; $i++)
		$password.=$string[rand(0, strlen($string)-1)];
	return $password;
}


function user_admin($args)
{
	global $user_id;

	if(isset($args['id']))
		return user_modify($args);
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title users">'._html_safe(USERS_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('user')))
	{
		print('<h2 class="title settings">Settings</h2>'."\n");
		$module = 'user';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title users">'._html_safe(USERS).'</h2>'."\n");
	$order = 'name';
	if(isset($args['sort']))
		switch($args['sort'])
		{
			case 'admin':	$order = 'admin';	break;
			case 'email':	$order = 'email';	break;
		}
	$users = _sql_array('SELECT user_id AS id, username AS name, enabled'
			.', admin, email FROM daportal_user'
			.' ORDER BY '.$order.' ASC');
	if(!is_array($users))
		return _error('Unable to list users');
	$count = count($users);
	for($i = 0; $i < $count; $i++)
	{
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'admin';
		$users[$i]['icon'] = 'icons/48x48/user.png';
		$users[$i]['thumbnail'] = 'icons/48x48/user.png';
		$users[$i]['name'] = _html_safe_link($users[$i]['name']);
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
		$users[$i]['email'] = '<a href="mailto:'.$users[$i]['email']
				.'">'._html_safe($users[$i]['email']).'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_USER, 'class' => 'new',
			'link' => 'index.php?module=user&action=new');
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
					'admin' => 'Admin', 'email' => EMAIL),
				'module' => 'user', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
				: 'name'));
}


function user_appearance($args)
{
	global $theme;

	print('<h1 class="title appearance">'._html_safe(APPEARANCE)."</h1>\n");
	$themes = array();
	if(($dir = opendir('themes')) != FALSE)
	{
		while(($de = readdir($dir)) != FALSE)
		{
			if(($len = strlen($de)) < 5
					|| substr($de, -4) != '.css')
				continue;
			$themes[] = substr($de, 0, -4);
		}
	}
	include('./modules/user/appearance.tpl');
}


function user_config_update($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!isset($args['user_register'])) /* XXX checkbox is not ticked */
		$args['user_register'] = SQL_FALSE;
	if(!isset($args['user_manual'])) /* XXX checkbox is not ticked */
		$args['user_manual'] = SQL_FALSE;
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^user_([a-zA-Z_]+)$', $k, $regs))
			_config_set('user', $regs[1], $args[$k], 0);
	header('Location: index.php?module=user&action=admin');
	_debug();
	exit(0);
}


function user_confirm($args)
{
	global $error, $info;

	include('./modules/user/user_confirm.tpl');
}


function user_default($args)
{
	global $user_id, $user_name;

	if(isset($args['id']))
		return user_display($args);
	if($user_id == 0)
		return user_login($args);
	if(($user = _sql_array('SELECT user_id, username, admin'
			.' FROM daportal_user'
			." WHERE user_id='$user_id'")) == FALSE)
		return _error('Invalid user');
	$user = $user[0];
	print('<h1 class="title home">'._html_safe($user['username'])
			."'s page</h1>\n");
	$modules = array();
	if(_user_admin($user_id) && ($d = _module_desktop('admin')) != FALSE)
		$modules[] = array('name' => $d['title'],
				'thumbnail' => 'icons/48x48/admin.png',
				'module' => 'admin', 'action' => 'default');
	$modules[] = array('name' => APPEARANCE,
			'thumbnail' => 'icons/48x48/appearance.png',
			'module' => 'user', 'action' => 'appearance');
	$modules[] = array('name' => MY_CONTENT,
			'thumbnail' => 'icons/48x48/content.png',
			'module' => 'user', 'action' => 'default',
			'id' => $user_id);
	$modules[] = array('name' => MY_PROFILE,
			'thumbnail' => 'icons/48x48/admin.png',
			'module' => 'user', 'action' => 'admin',
			'id' => $user_id);
	$modules[] = array('name' => LOGOUT,
			'thumbnail' => 'icons/48x48/logout.png',
			'module' => 'user', 'action' => 'logout');
	_module('explorer', 'browse', array('entries' => $modules,
				'view' => 'thumbnails', 'toolbar' => 0));
}


function user_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	_sql_query('DELETE FROM daportal_user_register'
			." WHERE user_id='".$args['id']."'");
	if(!_sql_query('DELETE FROM daportal_user'
			." WHERE user_id='".$args['id']."'"))
		return _error('Could not delete user');
}


function user_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!_sql_query('UPDATE daportal_user SET enabled='."'0'"
			." WHERE user_id='".$args['id']."'"))
		return _error('Could not disable user');
}


function user_display($args)
{
	if(!is_numeric($args['id']))
		return _error('Invalid user ID');
	$user = _sql_array('SELECT user_id AS id, username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['id']."'");
	if(!is_array($user) || count($user) != 1)
		return _error('Invalid user');
	$user = $user[0];
	$res = _sql_array('SELECT name FROM daportal_module ORDER BY name ASC');
	if(!is_array($res))
		return _error('Could not list modules');
	print('<h1 class="title user">'._html_safe($user['username'])
			."'s content</h1>\n");
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
			$e['thumbnail'] = 'icons/48x48/'.$e['icon'];
			$e['icon'] = 'icons/16x16/'.$e['icon'];
			$e['args'] = '&user_id='.$user['id'];
			$entries[] = $e;
		}
	}
	_module('explorer', 'browse', array('entries' => $entries,
				'view' => 'thumbnails', 'toolbar' => 0));
}


function user_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!_sql_query('UPDATE daportal_user SET enabled='."'1'"
			." WHERE user_id='".$args['id']."'"))
		return _error('Could not enable user');
}


function user_insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!ereg('^[a-z]{1,9}$', $args['username']))
		return _error('Username must be lower-case and no longer than'
				.' 9 characters', 1);
	if(strlen($args['password1']) < 1
			|| $args['password1'] != $args['password2'])
		return _error('Passwords must be non-empty and match', 1);
	$password = md5($args['password1']);
	if(!_sql_query('INSERT INTO daportal_user (username, password, enabled'
			.', admin, email) VALUES ('
			."'".$args['username']."', '$password'"
			.", '".(isset($args['enabled']) ? '1' : '0')."'"
			.", '".(isset($args['admin']) ? '1' : '0')."'"
			.", '".$args['email']."');"))
		return _error('Could not insert user');
	$id = _sql_id('daportal_user', 'user_id');
	user_display(array('id' => $id));
}


function user_login($args)
{
	global $user_id;

	if($user_id != 0)
		return user_default($args);
	$register = _config_get('user', 'register') == SQL_TRUE ? 1 : 0;
	$message = '';
	$username = '';
	if(isset($_POST['username']))
	{
		$message = WRONG_PASSWORD;
		$username = stripslashes($_POST['username']);
	}
	include('./modules/user/user_login.tpl');
}


function user_logout($args)
{
	global $user_id;

	if($user_id != 0)
		return _error('Unable to logout');
	return include('./modules/user/user_logout.tpl');
}


function user_modify($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$id = $args['id'];
	else if(!isset($args['id']) || $args['id'] == $user_id)
		$id = $user_id;
	else
		return _error(PERMISSION_DENIED);
	$admin = _user_admin($user_id) ? 1 : 0;
	$user = _sql_array('SELECT user_id, username, enabled, admin, email'
			.' FROM daportal_user WHERE user_id='."'$id'");
	if(!is_array($user) || count($user) != 1)
		return _error('Invalid user');
	$user = $user[0];
	$title = USER_MODIFICATION;
	include('./modules/user/user_update.tpl');
}


function user_new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = 'New user';
	$admin = 1;
	include('./modules/user/user_update.tpl');
}


function user_register($args)
{
	global $user_id;

	if(_config_get('user', 'register') != SQL_TRUE)
		return _error(PERMISSION_DENIED);
	if($user_id)
		return _error(ALREADY_LOGGED_IN);
	$message = '';
	if($_SERVER['REQUEST_METHOD'] == 'POST' && isset($args['username'])
			&& isset($args['email']))
	{
		if(!ereg('^[a-z]{1,9}$', $args['username']))
			$message = 'Username must be lower-case and no longer than 9 characters';
		else
		{
			if(_sql_single('SELECT username FROM daportal_user'
					." WHERE username='".$args['username']."'")
					== FALSE)
			{
				if(!ereg('^[a-zA-Z0-9_.-]+@[a-zA-Z0-9_.-]+\.'
						.'[a-zA-Z]{1,4}$',
						$args['email']))
					$message = EMAIL_INVALID;
				else if(_sql_array('SELECT email'
						.' FROM daportal_user'
						." WHERE email='".$args['email']."'")
						== FALSE)
					return _register_mail($args['username'],
							$args['email']);
				else
					$message = EMAIL_ALREADY_ASSIGNED;
			}
			else
				$message = USER_ALREADY_ASSIGNED;
		}
	}
	print('<h1 class="title user">'._html_safe(USER_REGISTRATION)."</h1>\n");
	if(strlen($message))
		_error($message, 1);
	include('./modules/user/user_register.tpl');
}

function _register_mail($username, $email)
{
	$password = _password_new();
	_info('New password is: '.$password);
	if(_sql_query('INSERT INTO daportal_user (username, password, enabled'
			.', admin, email) VALUES ('
			."'$username', '".md5($password)."', '0', '0', '"
			.$email."')") == FALSE)
		return _error('Could not insert user');
	$id = _sql_id('daportal_user', 'user_id');
	include('./modules/user/user_pending.tpl');
	_password_mail($id, $username, $email, $password);
}


function _system_confirm($key)
{
	global $error;

	$error = 'Confirmation failed';
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
	if(_config_get('user', 'manual') == SQL_FALSE)
		return _confirm_auto($key, $user);
	return _confirm_manual($key, $user);
}

function _confirm_auto($key, $user)
{
	if(_sql_query('UPDATE daportal_user SET enabled='."'1'"
			." WHERE user_id='".$user['user_id']."'") == FALSE)
		return _error('Could not enable user');
	if(_sql_query('DELETE FROM daportal_user_register WHERE key='."'$key'")
			== FALSE)
		_error('Could not remove registration key');
	if(strlen(session_id()) == 0)
		session_start();
	$_SESSION['user_id'] = $user['user_id'];
	$_SESSION['user_name'] = $user['username'];
	header('Location: index.php?module=user');
	exit(0);
}

function _confirm_manual($key, $user)
{
	global $info;

	if(_sql_query('DELETE FROM daportal_user_register WHERE key='."'$key'")
			== FALSE)
		_error('Could not remove registration key');
	if(_sql_query('DELETE FROM daportal_user_register WHERE key='."'$key'")
			== FALSE)
		_error('Could not remove registration key');
	$info = 'Your confirmation was acknowledged by the system. Your account'
			.' request will be manually enabled shortly. Thanks!';
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
		.'https://'.$_SERVER['HTTP_HOST'].$_SERVER['SCRIPT_NAME']
		.'?module=user&action=admin&id='.$user['user_id']."\n";
	require_once('./system/mail.php');
	_mail('Administration Team', $to, $subject, $content);
}


function _system_login()
{
	global $user_id; 

	if(strlen(session_id()) == 0)
		session_start();
	$password = md5($_POST['password']);
	$res = _sql_array('SELECT user_id, username, admin FROM daportal_user'
			.' WHERE username='."'".$_POST['username']."'"
			.' AND password='."'$password' AND enabled='1'");
	if(!is_array($res) || count($res) != 1)
		return _error('Unable to login', 0);
	$res = $res[0];
	$_SESSION['user_id'] = $res['user_id'];
	$_SESSION['user_name'] = $res['username'];
	header('Location: index.php?module=user');
	exit(0);
}

function _system_logout()
{
	global $user_id, $user_name;

	$_SESSION['user_id'] = 0;
	$_SESSION['user_name'] = 'Anonymous';
	unset($_SESSION['debug']);
	unset($_SESSION['theme']);
	$user_id = $_SESSION['user_id'];
	$user_name = $_SESSION['user_name'];
}

function user_system($args)
{
	global $html;

	if($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['action']))
	{
		if($_POST['action'] == 'login')
			_system_login();
		else if($_POST['action'] == 'config_update')
			$html = 0;
		else if($_POST['action'] == 'error')
			$_POST['action'] = 'default';
		else if($_POST['action'] == 'appearance')
			return _system_appearance($args);
	}
	else if($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['action']))
	{
		if($_GET['action'] == 'logout')
			_system_logout();
		else if($_GET['action'] == 'confirm' && isset($args['key']))
			_system_confirm($args['key']);
		else if($_GET['action'] == 'error')
			$_GET['action'] = 'default';
	}
}

function _system_appearance($args)
{
	unset($_SESSION['theme']);
	if(isset($args['theme']) && strchr($args['theme'], '/') == FALSE
			&& is_readable('themes/'.$args['theme'].'.css'))
		$_SESSION['theme'] = $args['theme'];
	header('Location: index.php?module=user&action=appearance');
}


function user_update($args)
{
	global $user_id;

	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$id = $args['id'];
	else if($user_id != 0)
		$id = $user_id;
	else
		return _error(PERMISSION_DENIED);
	$password = '';
	if(strlen($args['password1'])
			&& $args['password1'] == $args['password2'])
		$password = "password='".md5($args['password1'])."'";
	if(_user_admin($user_id))
	{
		if(!_sql_query('UPDATE daportal_user SET'
					." username='".$args['username']."'"
					.", enabled='".(isset($args['enabled'])
						&& $args['enabled'] == 'on'
						? '1' : '0')."'"
					.", admin='".(isset($args['admin'])
						&& $args['admin'] == 'on'
						? '1' : '0')."'"
					.", email='".$args['email']."'"
					.(strlen($password) ? ', '.$password
					       	: '')." WHERE user_id='$id'"))
			return _error('Could not update user');
	}
	else if(strlen($password) && !_sql_query('UPDATE daportal_user SET '
				.$password." WHERE user_id='$id'"))
		return _error('Could not update user');
	user_display(array('id' => $id));
}

?>
