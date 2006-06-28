<?php //config.php


//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: index.php'));


$template = 'DaPortal';
$theme = 'DaPortal';
$debug = 1;
$dbtype = 'pgsql';	//or 'mysql'
//$dbhostname = '';	//defaults to local connections
$dbname = 'daportal';
$dbuser = 'daportal';
$dbpassword = 'daportal';

?>
