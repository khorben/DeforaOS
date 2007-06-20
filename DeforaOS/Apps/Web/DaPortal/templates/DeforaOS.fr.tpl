<?php //$Id$

//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../index.php'));

$text['DEVELOPMENT'] = 'Développement';
$text['DOWNLOAD'] = 'Télécharger';
$text['POLICY'] = 'Objectifs';
$text['PROJECT'] = 'Projet';
$text['PROJECTS'] = 'Projets';
$text['REPORTS'] = 'Rapports';
$text['ROADMAP'] = 'Progression';

?>
