<?php //config.php


//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: index.php'));


$template = 'DaPortal';
$theme = 'DaPortal';
//$debug = 1;		//force debugging on
$dbtype = 'pgsql';	//or 'mysql' or 'sqlite'
//$dbhostname = '';	//defaults to local connections
$dbname = 'daportal';
$dbuser = 'daportal';
$dbpassword = 'daportal';
$friendlylinks = 0;	//force "friendly links" generation
//$friendlykicker = '';	//replaces "index.php" in "friendly links"

?>
