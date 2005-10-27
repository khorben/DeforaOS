<?php
//modules/top/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Top links';
$admin = 1;
$list = 0;


?>
