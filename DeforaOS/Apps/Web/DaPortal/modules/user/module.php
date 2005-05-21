<?php
//modules/user/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function _admin_modify($id)
{
	if(!is_numeric($id))
		return _error('Invalid user ID');
	include('user_update.tpl');
}


function user_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(isset($args['id']))
		return _admin_modify($args['id']);
	print('<h1><img src="module/user/icon.png" alt=""/> Users administration</h1>'."\n");
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
		$users[$i]['icon'] = 'modules/user/icon.png';
		$users[$i]['thumbnail'] = 'modules/user/icon.png';
	}
	_module('explorer', 'browse', array('entries' => $users));
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
