<?php //modules/explorer/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Explorer';
$admin = 0;
$list = 0;


?>
