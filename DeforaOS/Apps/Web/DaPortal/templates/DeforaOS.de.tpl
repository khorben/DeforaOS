<?php //$Id$

//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../index.php'));

$text['PROJECT'] = 'Projekt';
$text['PROJECTS'] = 'Projekte';

?>
