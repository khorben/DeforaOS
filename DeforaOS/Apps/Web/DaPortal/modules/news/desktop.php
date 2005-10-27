<?php
//modules/news/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = NEWS;
$admin = 1;
$list = 1;
if($user_id)
	$actions = array('submit' => 'Submit');


?>
