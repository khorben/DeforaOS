<?php
//modules/user/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function _modify($id)
{
	global $user_id;

	if(!is_numeric($id))
		return _error('Invalid user ID');
	require_once('system/user.php');
	$admin = _user_admin($user_id) ? 1 : 0;
	$super = ($id == $user_id) ? 1 : 0;
	$user = _sql_array('SELECT username, admin FROM daportal_user'
			." WHERE user_id='$id';");
	if(!is_array($user) || count($user) != 1)
		return _error('Invalid user');
	$user = $user[0];
	$title = 'User modification';
	include('user_update.tpl');
}


function user_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(isset($args['id']))
		return _modify($args['id']);
	print('<h1><img src="modules/user/icon.png" alt=""/> Users administration</h1>'."\n");
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.' FROM daportal_user'
			.' ORDER BY name ASC;');
	if(!is_array($users))
		return _error('Unable to list users');
	$count = count($users);
	for($i = 0; $i < $count; $i++)
	{
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'admin';
		$users[$i]['icon'] = 'modules/user/user.png';
		$users[$i]['thumbnail'] = 'modules/user/user.png';
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New user',
			'icon' => 'modules/user/icon.png',
			'link' => 'index.php?module=user&action=new');
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $users));
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


function user_insert($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!ereg('^[a-z]{1,9}$', $args['username']))
		return _error('Username must be lower-case and no longer than 9 characters');
	if(strlen($args['password1']) < 1
			|| $args['password1'] != $args['password2'])
		return _error('Passwords must be non-empty and match', 1);
	$password = md5($args['password1']);
	if(!_sql_query('INSERT INTO daportal_user (username, password) VALUES ('
			."'".$args['username']."'"
			.", '$password');"))
		return _error('Could not insert user');
	$id = _sql_id('daportal_user', 'user_id');
	user_display(array('id' => $id));
}


function user_login($args)
{
	global $user_id;

	if($user_id != 0)
		return user_default($args);
	if(isset($_POST['username']))
	{
		$message = 'Wrong password';
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

	if($args['id'] != $user_id)
		return _error('Permission denied');
	_modify($args['id']);
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


function _system_login()
{
	global $user_id; 

	$password = md5($_POST['password']);
	if(($res = _sql_array('SELECT user_id, admin FROM daportal_user'
			.' WHERE username='."'".$_POST['username']."'"
			.' and password='."'$password';")) == FALSE)
		return _error('Unable to login');
	$res = $res[0];
	$_SESSION['user_id'] = $res['user_id'];
	$_SESSION['user_name'] = stripslashes($_POST['username']);
}

function _system_logout()
{
	global $user_id, $user_name;

	$_SESSION['user_id'] = 0;
	$_SESSION['user_name'] = 'Anonymous';
}

function user_system($args)
{
	global $user_id, $user_name;

	if($_SERVER['REQUEST_METHOD'] == 'POST' && $_POST['action'] == 'login')
		_system_login();
	else if($_SERVER['REQUEST_METHOD'] == 'GET'
			&& $_GET['action'] == 'logout')
		_system_logout();
	$user_id = $_SESSION['user_id'];
	$user_name = $_SESSION['user_name'];
}

?>
