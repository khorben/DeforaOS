<?php //modules/news/desktop.php



//check url
if(strcmp($_SERVER['SCRIPT_NAME'], $_SERVER['PHP_SELF']) != 0
		|| !ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


$title = NEWS;
$admin = 1;
$list = 1;
global $user_id;
if($user_id)
	$actions = array('submit' => SUBMIT);


?>
