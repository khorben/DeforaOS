<?php
//system/lang.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

if(ereg('^([a-zA-Z]+)', $_SERVER['HTTP_ACCEPT_LANGUAGE'], $regs))
	$lang = $regs[1];

?>
