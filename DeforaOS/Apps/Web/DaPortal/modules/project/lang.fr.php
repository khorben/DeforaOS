<?php //modules/project/lang.fr.php

//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

$text['ASSIGNED_TO'] = 'Affect� �';
$text['BROWSE_SOURCE'] = 'Parcourir les sources';
$text['BUG_REPORTS'] = 'Rapports de bugs';
$text['FILES'] = 'Fichiers';
$text['INVALID_PROJECT'] = 'Projet non valide';
$text['MEMBERS'] = 'Membres';
$text['NEW_PROJECT'] = 'Nouveau projet';
$text['NO_CVS_REPOSITORY'] = "Ce projet n'est pas g�r� par CVS";
$text['PRIORITY'] = 'Priorit�';
$text['PROJECT'] = 'Projet';
$text['PROJECT_LIST'] = 'Liste des projets';
$text['PROJECT_NAME'] = 'Nom du projet';
$text['PROJECTS'] = 'Projets';
$text['PROJECTS_ADMINISTRATION'] = 'Administration des projets';
$text['REPLY_BY'] = 'R�ponse par';
$text['REPLY_ON'] = 'le';
$text['REPORT_A_BUG'] = 'Rapporter un bug';
$text['REPORT_BUG_FOR'] = 'Rapporter un bug pour';
$text['STATE'] = 'Etat';
$text['STATE_CHANGED_TO'] = 'Etat chang� �';
$text['SUBMITTER'] = 'Envoy� par';
$text['TIMELINE'] = 'Progression';

?>
