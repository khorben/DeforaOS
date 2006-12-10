<?php //modules/user/desktop.php


$title = 'Users';
$admin = 1;
$list = 1;
global $user_id;
if($user_id != 0)
{
	$actions = array('logout' => 'Logout');
	require_once('./system/user.php');
	$title = _user_admin($user_id) ? 'Users' : "User's page";
}
else
	$actions = array('login' => 'Login');

global $lang;
if($lang == 'de')
{
	$title = 'Benutzer';
	if(isset($actions['login']))
		$actions['login'] = 'Einloggen';
	if(isset($actions['logout']))
		$actions['logout'] = 'Ausloggen';
}
else if($lang == 'fr')
{
	$title = 'Utilisateurs';
	if(isset($actions['login']))
		$actions['login'] = 'Authentification';
	if(isset($actions['logout']))
		$actions['logout'] = 'Déconnexion';
}

?>
