<?php
//modules/admin/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Administration';
$admin = 1;
$list = 0;
global $user_id;
require_once('system/user.php');
if(_user_admin($user_id))
{
	$list = 1;
	$actions = array('content' => CONTENT,
			'module' => 'Modules',
			'site' => 'Sites');
}

?>
