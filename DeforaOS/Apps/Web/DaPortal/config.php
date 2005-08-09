<?php


//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: index.php'));


$title = 'DaPortal';
$template = 'DaPortal';
$theme = 'DaPortal';
$debug = 1;
$dbtype = 'pgsql';
$dbuser = 'daportal';
$dbpassword = 'daportal';

//$lang = 'en';

?>
