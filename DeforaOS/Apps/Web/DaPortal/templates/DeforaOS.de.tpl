<?php

//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

$text['PROJECT'] = 'Projekt';
$text['PROJECTS'] = 'Projekte';

?>
