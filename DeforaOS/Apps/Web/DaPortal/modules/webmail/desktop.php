<?php //modules/webmail/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Webmail';
$admin = 1;
$list = 1;
$actions = array('default' => 'Message list',
		'logout' => 'Logout');
global $lang;
if($lang == 'de')
{
}
else if($lang == 'fr')
{
	$actions = array('default' => 'Boite de réception',
			'logout' => 'Déconnexion');
}


?>
