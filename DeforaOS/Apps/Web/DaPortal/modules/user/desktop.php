<?php
//modules/user/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Users';
$admin = 1;
$list = 1;
global $user_id;
if($user_id != 0)
{
	$actions = array('logout' => LOGOUT);
	$title = "User's page";
}
else
	$actions = array('login' => LOGIN);
global $lang;
if($lang == 'fr')
	$title = 'Utilisateurs';


?>
