<?php //config.php


//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: index.php'));


$template = 'DeforaOS';
$theme = 'DeforaOS';
//$debug = 1;		//force debugging on
$dbtype = 'sqlite';	//or 'mysql' or 'sqlite'
//$dbhostname = '';	//defaults to local connections
$dbname = '/var/httpd/sqlite/daportal.db';
$dbuser = 'daportal';
$dbpassword = 'daportal';
$friendlylinks = 1;	//force "friendly links" generation

?>
