<?php


//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: index.php'));


$template = 'DaPortal';
$theme = 'DaPortal';
$debug = 1;
$dbtype = 'pgsql';
$dbuser = 'daportal';
$dbpassword = 'daportal';

?>
