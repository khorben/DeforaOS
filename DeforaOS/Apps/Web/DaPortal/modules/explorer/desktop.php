<?php
//modules/explorer/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Explorer';
$list = 0;


?>
