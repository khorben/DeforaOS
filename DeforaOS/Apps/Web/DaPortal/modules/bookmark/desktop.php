<?php //modules/bookmark/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Bookmarks';
$admin = 1;
$list = 1;

?>
