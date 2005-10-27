<?php
//modules/project/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Monitoring';
$admin = 1;
$list = 1;
$actions = array('host_list' => 'Hosts');

?>
