<?php //$Id$

//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../index.php'));

$text['PROJECT'] = 'Projekt';
$text['PROJECTS'] = 'Projekte';

?>
