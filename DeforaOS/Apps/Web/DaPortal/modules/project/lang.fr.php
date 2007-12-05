<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));

$text['ASSIGNED_TO'] = 'Affecté à';
$text['BROWSE_SOURCE'] = 'Parcourir les sources';
$text['BUG_REPORT'] = 'Rapport de bug';
$text['BUG_REPORTS'] = 'Rapports de bugs';
$text['BUGS_BY'] = 'Bugs par';
$text['CVS_PATH'] = 'Chemin CVS';
$text['FILES'] = 'Fichiers';
$text['INVALID_PROJECT'] = 'Projet non valide';
$text['MEMBERS'] = 'Membres';
$text['MODIFICATION_OF'] = 'Modification de';
$text['NEW_PROJECT'] = 'Nouveau projet';
$text['NO_CVS_REPOSITORY'] = "Ce projet n'est pas géré par CVS";
$text['PRIORITY'] = 'Priorité';
$text['PROJECT'] = 'Projet';
$text['PROJECT_LIST'] = 'Liste des projets';
$text['PROJECT_NAME'] = 'Nom du projet';
$text['PROJECTS'] = 'Projets';
$text['PROJECTS_ADMINISTRATION'] = 'Administration des projets';
$text['PROJECTS_REGISTERED'] = 'projets enregistrés';
$text['RELEASES'] = 'Versions';
$text['REPLY_BY'] = 'Réponse par';
$text['REPLY_ON'] = 'le';
$text['REPORT_A_BUG'] = 'Rapporter un bug';
$text['REPORT_BUG_FOR'] = 'Rapporter un bug pour';
$text['REVISION'] = 'Révision';
$text['SCREENSHOTS'] = "Captures d'écran";
$text['SETTINGS'] = 'Configuration';
$text['STATE'] = 'Etat';
$text['STATE_CHANGED_TO'] = 'Etat changé à';
$text['STATISTICS'] = 'Statistiques';
$text['SUBMITTER'] = 'Envoyé par';
$text['THERE_ARE'] = 'Il y a';
$text['TIMELINE'] = 'Progression';

?>
