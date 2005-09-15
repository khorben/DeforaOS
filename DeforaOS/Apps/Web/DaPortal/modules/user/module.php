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
	$user = _sql_array('SELECT user_id, username, admin, email'
			.' FROM daportal_user'
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
	$order = 'name ASC';
	switch($args['sort'])
	{
		case 'admin':	$order = 'admin ASC'; break;
		case 'email':	$order = 'email ASC'; break;
	}
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.', admin, email'
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
		$users[$i]['apply_module'] = 'user';
		$users[$i]['apply_id'] = $users[$i]['id'];
		$users[$i]['admin'] = $users[$i]['admin'] == 't' ? YES : NO;
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'New user',
			'icon' => 'modules/user/icon.png',
			'link' => 'index.php?module=user&action=new');
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Delete',
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete',
			'confirm' => 'delete');
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $users,
				'class' => array('admin' => 'Admin',
					'email' => 'e-mail'),
				'module' => 'user',
				'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
				: 'name',
				'view' => 'details'));
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


function user_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	if(!_sql_query('DELETE FROM daportal_user'
			." WHERE user_id='".$args['id']."';"))
		return _error('Could not delete user');
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
	if(!_sql_query('INSERT INTO daportal_user (username, password, email'
			.') VALUES ('
			."'".$args['username']."'"
			.", '$password', '".$args['email']."');"))
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
		return _error('Unable to login', 0);
	$res = $res[0];
	$_SESSION['user_id'] = $res['user_id'];
	$_SESSION['user_name'] = stripslashes($_POST['username']);
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
	if($_SERVER['REQUEST_METHOD'] == 'POST' && $_POST['action'] == 'login')
		_system_login();
	else if($_SERVER['REQUEST_METHOD'] == 'GET'
			&& $_GET['action'] == 'logout')
		_system_logout();
}


function user_update($args)
{
	global $user_id;

	require_once('system/user.php');
	//FIXME should also allow user to update some of his own details
	if(!_user_admin($user_id))
		return _error('Permission denied');
	//FIXME check if password is set and matches
	if(!_sql_query('UPDATE daportal_user SET '
			."admin='".(isset($args['admin']) ? '1' : '0')."'"
			." WHERE user_id='".$args['user_id']."';"))
		return _error('Could not update user');
	user_display(array('id' => $args['user_id']));
}

?>
