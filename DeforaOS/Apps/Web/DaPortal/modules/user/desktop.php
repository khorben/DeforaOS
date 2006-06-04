<?php //modules/user/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$text['USERS'] = 'Users';
global $lang;
if($lang == 'de')
	$text['USERS'] = 'Benutzer';
else if($lang == 'fr')
	$text['USERS'] = 'Utilisateurs';
_lang($text);

$title = USERS;
$admin = 1;
$list = 1;
global $user_id;
if($user_id != 0)
{
	$actions = array('logout' => LOGOUT);
	require_once('./system/user.php');
	$title = _user_admin($user_id) ? USERS : "User's page";
}
else
	$actions = array('login' => LOGIN);

?>
