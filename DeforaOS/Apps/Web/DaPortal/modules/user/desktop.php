<?php
//modules/user/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'User';
$list = 1;
global $user_id;
if($user_id != 0)
	$actions = array('logout' => 'Logout');
else
	$actions = array('login' => 'Login');


?>
