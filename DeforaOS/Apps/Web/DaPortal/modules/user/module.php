<?php //modules/user/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['EMAIL_ALREADY_ASSIGNED'] = 'e-mail already assigned';
$text['EMAIL_INVALID'] = 'e-mail is not valid';
$text['REGISTER'] = 'Register';
$text['USER_ALREADY_ASSIGNED'] = 'Username already assigned';
$text['USER_LOGIN'] = 'User login';
$text['USER_REGISTRATION'] = 'User registration';
$text['WRONG_PASSWORD'] = 'Wrong password';
global $lang;
if($lang == 'fr')
{
	$text['EMAIL_ALREADY_ASSIGNED'] = 'Cet e-mail est déjà utilisé';
	$text['EMAIL_INVALID'] = "Cet e-mail n'est pas valide";
	$text['REGISTER'] = "S'inscrire";
	$text['USER_ALREADY_ASSIGNED'] = 'Cet utilisateur existe déjà';
	$text['USER_LOGIN'] = 'Authentification utilisateur';
	$text['USER_REGISTRATION'] = 'Inscription utilisateur';
	$text['WRONG_PASSWORD'] = 'Mot de passe incorrect';
}
_lang($text);


function _password_mail($id, $username, $email, $password = FALSE)
	/* FIXME weak passwords and keys...? */
{
	if($password == FALSE)
		$password = _password_new();
	$key = md5(_password_new());
	if(_sql_query('INSERT INTO daportal_user_register (user_id, key)'
			.' VALUES ('
			."'$id', '$key');") == FALSE)
		return _error('Could not create confirmation key');
	$message = "Your password is '$password'\n\n"
			."Please click on the following link to confirm:\n"
			.'https://'.$_SERVER['SERVER_NAME']
			.$_SERVER['SCRIPT_NAME']
			.'?module=user&action=confirm&key='.$key;
	$headers = 'From: DeforaOS Administration Team <'
			.$_SERVER['SERVER_ADMIN'].">\n";
	_info('Mail sent to: '.$username.' <'.$email.'>');
	if(mail($username.' <'.$email.'>', 'User confirmation', $message,
		$headers) == FALSE)
		return _error('Could not send mail');
}


function _password_new()
{
	$string = 'abcdefghijklmnopqrstuvwxyz'
			.'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
			.'0123456789';
	$password = '';
	for($i = 0; $i < 8; $i++)
		$password.=$string[rand(0, strlen($string))];
	return $password;
}


function user_admin($args)
{
	global $user_id;

	if(isset($args['id']))
		return user_modify($args);
	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/user/icon.png" alt=""/>'
			.' Users administration</h1>'."\n");
	if(($configs = _config_list('user')))
	{
		print('<h2><img src="modules/admin/icon.png" alt=""/>'
			.' Configuration</h2>'."\n");
		$module = 'user';
		$action = 'config_update';
		include('system/config.tpl');
	}
	$order = 'name ASC';
	switch($args['sort'])
	{
		case 'admin':	$order = 'admin ASC'; break;
		case 'email':	$order = 'email ASC'; break;
	}
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.', enabled, admin, email'
			.' FROM daportal_user'
			.' ORDER BY '.$order.';');
	if(!is_array($users))
		return _error('Unable to list users');
	$count = count($users);
	for($i = 0; $i < $count; $i++)
	{
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'admin';
		$users[$i]['icon'] = 'modules/user/user.png';
		$users[$i]['thumbnail'] = 'modules/user/user.png';
		$users[$i]['name'] = _html_safe_link($users[$i]['name']);
		$users[$i]['apply_module'] = 'user';
		$users[$i]['apply_id'] = $users[$i]['id'];
		$users[$i]['enabled'] = $users[$i]['enabled'] == 't'
			? 'enabled' : 'disabled';
		$users[$i]['enabled'] = '<img src="icons/16x16/'
				.$users[$i]['enabled'].'.png" alt="'
				.$users[$i]['enabled'].'" title="'
				.($users[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED)
				.'"/>';
		$users[$i]['admin'] = $users[$i]['admin'] == 't'
			? 'enabled' : 'disabled';
		$users[$i]['admin'] = '<img src="icons/16x16/'
				.$users[$i]['admin'].'.png" alt="'
				.$users[$i]['admin'].'" title="'
				.($users[$i]['admin'] == 'enabled'
						? ENABLED : DISABLED)
				.'"/>';
		$users[$i]['email'] = '<a href="mailto:'.$users[$i]['email']
				.'">'._html_safe($users[$i]['email']).'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New user',
			'icon' => 'modules/user/icon.png',
			'link' => 'index.php?module=user&action=new');
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Disable',
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable');
	$toolbar[] = array('title' => 'Enable',
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable');
	$toolbar[] = array('title' => 'Delete',
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete',
			'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('toolbar' => $toolbar,
				'entries' => $users,
				'class' => array('enabled' => ENABLED,
					'admin' => 'Admin',
					'email' => 'e-mail'),
				'module' => 'user',
				'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
				: 'name',
				'view' => 'details'));
}


function user_config_update($args)
{
	global $user_id, $module_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^user_([a-zA-Z_]+)$', $k, $regs))
			_config_set('user', $regs[1], $args[$k], 0);
	header('Location: index.php?module=user&action=admin');
	exit(0);
}


function user_confirm($args)
{
	include('user_confirm.tpl');
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
			." WHERE user_id='$user_id';")) == FALSE)
		return _error('Invalid user');
	$user = $user[0];
	include('user_homepage.tpl');
}


function user_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	_sql_query('DELETE FROM daportal_user_register'
			." WHERE user_id='".$args['id']."';");
	if(!_sql_query('DELETE FROM daportal_user'
			." WHERE user_id='".$args['id']."';"))
		return _error('Could not delete user');
}


function user_disable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!_sql_query('UPDATE daportal_user SET'
			." enabled='f'"
			." WHERE user_id='".$args['id']."';"))
		return _error('Could not disable user');
}


function user_display($args)
{
	if(!is_numeric($args['id']))
		return _error('Invalid user ID');
	if(($user = _sql_array('SELECT user_id, username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['id']."';")) == FALSE)
		return _error('Invalid user');
	$user = $user[0];
	include('user_display.tpl');
}


function user_enable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!_sql_query('UPDATE daportal_user SET'
			." enabled='t'"
			." WHERE user_id='".$args['id']."';"))
		return _error('Could not enable user');
}


function user_insert($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!ereg('^[a-z]{1,9}$', $args['username']))
		return _error('Username must be lower-case and no longer than 9 characters', 1);
	if(strlen($args['password1']) < 1
			|| $args['password1'] != $args['password2'])
		return _error('Passwords must be non-empty and match', 1);
	$password = md5($args['password1']);
	if(!_sql_query('INSERT INTO daportal_user (username, password, enabled'
			.', admin, email'
			.') VALUES ('
			."'".$args['username']."'"
			.", '$password'"
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
	$register = _config_get('user', 'register') == 't' ? 1 : 0;
	if(isset($_POST['username']))
	{
		$message = WRONG_PASSWORD;
		$username = stripslashes($_POST['username']);
	}
	include('user_login.tpl');
}


function user_logout($args)
{
	global $user_id;

	if($user_id != 0)
		return _error('Unable to logout');
	return include('user_logout.tpl');
}


function user_modify($args)
{
	global $user_id;

	if($user_id == 0)
		return _error('Permission denied');
	require_once('system/user.php');
	if(_user_admin($user_id))
		$id = $args['id'];
	else if(!isset($args['id']) || $args['id'] == $user_id)
		$id = $user_id;
	else
		return _error('Permission denied');
	$admin = _user_admin($user_id) ? 1 : 0;
	$user = _sql_array('SELECT user_id, username, enabled, admin, email'
			.' FROM daportal_user'
			." WHERE user_id='$id';");
	if(!is_array($user) || count($user) != 1)
		return _error('Invalid user');
	$user = $user[0];
	$title = 'User modification';
	include('user_update.tpl');
}


function user_new($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	$title = 'New user';
	$admin = 1;
	include('user_update.tpl');
}


function user_register($args)
{
	global $user_id;

	if(_config_get('user', 'register') != 't')
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
					." WHERE username='".$args['username']."';")
					== FALSE)
			{
				if(!ereg('^[a-zA-Z0-9_.-]+@[a-zA-Z0-9_.-]+\.'
						.'[a-zA-Z]{1,4}$',
						$args['email']))
					$message = EMAIL_INVALID;
				else if(_sql_array('SELECT email'
						.' FROM daportal_user'
						." WHERE email='".$args['email']."';")
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
	print('<h1><img src="modules/user/icon.png" alt=""/> '
			._html_safe(USER_REGISTRATION).'</h1>');
	if(strlen($message))
		_error($message, 1);
	include('user_register.tpl');
}

function _register_mail($username, $email)
{
	$password = _password_new();
	_info('New password is: '.$password);
	if(_sql_query('INSERT INTO daportal_user (username, password, enabled'
			.', admin, email) VALUES ('
			."'$username', '".md5($password)."', '0', '0', '"
			.$email."');")
			== FALSE)
		return _error('Could not insert user');
	$id = _sql_id('daportal_user', 'user_id');
	include('user_pending.tpl');
	_password_mail($id, $username, $email, $password);
}


function _system_confirm($key)
{
	/* FIXME remove expired registration keys */
	/* FIXME use a transaction */
	$user = _sql_array('SELECT daportal_user.user_id'
			.', daportal_user.username'
			.' FROM daportal_user, daportal_user_register'
			.' WHERE daportal_user.user_id'
			.'=daportal_user_register.user_id'
			." AND key='$key';");
	if(!is_array($user) || count($user) != 1)
		return;
	$user = $user[0];
	if(_sql_query('UPDATE daportal_user SET'
			." enabled='t' WHERE user_id='".$user['user_id']."';")
			== FALSE)
		return _error('Could not enable user');
	if(_sql_query('DELETE FROM daportal_user_register'
			." WHERE key='$key';") == FALSE)
		_error('Could not remove registration key');
	$_SESSION['user_id'] = $user['user_id'];
	$_SESSION['user_name'] = $user['username'];
	header('Location: index.php?module=user');
	exit(0);
}


function _system_login()
{
	global $user_id; 

	$password = md5($_POST['password']);
	$res = _sql_array('SELECT user_id, username, admin FROM daportal_user'
			.' WHERE username='."'".$_POST['username']."'"
			.' AND password='."'$password'"
			." AND enabled='t';");
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
	$user_id = $_SESSION['user_id'];
	$user_name = $_SESSION['user_name'];
}

function user_system($args)
{
	global $html;

	if($_SERVER['REQUEST_METHOD'] == 'POST')
	{
		if($_POST['action'] == 'login')
			_system_login();
		else if($_POST['action'] == 'config_update')
			$html = 0;
	}
	else if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		if($_GET['action'] == 'logout')
			_system_logout();
		else if($_GET['action'] == 'confirm')
			_system_confirm($args['key']);
	}
}


function user_update($args)
{
	global $user_id;

	require_once('system/user.php');
	if(_user_admin($user_id))
		$id = $args['id'];
	else if($user_id != 0)
		$id = $user_id;
	else
		return _error('Permission denied');
	$password = '';
	if(strlen($args['password1'])
			&& $args['password1'] == $args['password2'])
		$password = "password='".md5($args['password1'])."'";
	if(_user_admin($user_id))
	{
		if(!_sql_query('UPDATE daportal_user SET'
					." username='".$args['username']."'"
					.", enabled='".($args['enabled'] == 'on'
						? '1' : '0')."'"
					.", admin='".($args['admin'] == 'on'
						? '1' : '0')."'"
					.", email='".$args['email']."'"
					.', '.$password
					." WHERE user_id='$id';"))
			return _error('Could not update user');
	}
	else if(strlen($password) && !_sql_query('UPDATE daportal_user SET '
				.$password." WHERE user_id='$id';"))
		return _error('Could not update user');
	user_display(array('id' => $id));
}

?>
