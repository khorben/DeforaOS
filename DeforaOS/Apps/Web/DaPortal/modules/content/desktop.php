<?php //modules/content/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Contents';
$admin = 1;
$list = 0;

?>
