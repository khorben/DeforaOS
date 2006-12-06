<?php //modules/project/lang.fr.php

//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

$text['ASSIGNED_TO'] = 'Affecté à';
$text['BROWSE_SOURCE'] = 'Parcourir les sources';
$text['BUG_REPORTS'] = 'Rapports de bugs';
$text['FILES'] = 'Fichiers';
$text['INVALID_PROJECT'] = 'Projet non valide';
$text['MEMBERS'] = 'Membres';
$text['NEW_PROJECT'] = 'Nouveau projet';
$text['NO_CVS_REPOSITORY'] = "Ce projet n'est pas géré par CVS";
$text['PRIORITY'] = 'Priorité';
$text['PROJECT'] = 'Projet';
$text['PROJECT_LIST'] = 'Liste des projets';
$text['PROJECT_NAME'] = 'Nom du projet';
$text['PROJECTS'] = 'Projets';
$text['PROJECTS_ADMINISTRATION'] = 'Administration des projets';
$text['RELEASES'] = 'Versions';
$text['REPLY_BY'] = 'Réponse par';
$text['REPLY_ON'] = 'le';
$text['REPORT_A_BUG'] = 'Rapporter un bug';
$text['REPORT_BUG_FOR'] = 'Rapporter un bug pour';
$text['REVISION'] = 'Révision';
$text['SCREENSHOTS'] = "Captures d'écran";
$text['STATE'] = 'Etat';
$text['STATE_CHANGED_TO'] = 'Etat changé à';
$text['SUBMITTER'] = 'Envoyé par';
$text['TIMELINE'] = 'Progression';

?>
