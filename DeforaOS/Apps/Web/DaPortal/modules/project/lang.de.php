<?php //modules/project/lang.de.php

//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

$text['BROWSE_SOURCE'] = 'Quellcode betrachten';
$text['NEW_PROJECT'] = 'Neues projekt';
$text['PRIORITY'] = 'Priorität';
$text['PROJECT'] = 'Projekt';
$text['PROJECT_LIST'] = 'Projektliste';
$text['PROJECTS'] = 'Projekte';
$text['STATE'] = 'Stand';
$text['TIMELINE'] = 'Fortschritt';

?>
