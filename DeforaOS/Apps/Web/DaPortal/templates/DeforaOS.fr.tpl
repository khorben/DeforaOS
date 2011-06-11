<?php //$Id$

//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../index.php'));

$text['DEVELOPMENT'] = 'Développement';
$text['DOWNLOAD'] = 'Télécharger';
$text['POLICY'] = 'Objectifs';
$text['PROJECT'] = 'Projet';
$text['PROJECTS'] = 'Projets';
$text['REPORTS'] = 'Rapports';
$text['ROADMAP'] = 'Progression';

?>
