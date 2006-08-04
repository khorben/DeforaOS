<?php //modules/download/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = DOWNLOADS;
$admin = 1;
$list = 1;

?>
