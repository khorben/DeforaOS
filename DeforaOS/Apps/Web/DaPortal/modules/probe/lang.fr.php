<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));


//lang
$text['COMMENT'] = 'Commentaire';
$text['CONFIGURATION_ERROR'] = 'Erreur de configuration';
$text['GRAPH_LIST'] = 'Liste des graphes';
$text['HOST_LIST'] = 'Liste des machines';
$text['LOAD_AVERAGE'] = 'Charge moyenne';
$text['LOGGED_USERS'] = 'Utilisateurs connectés';
$text['MEMORY_USAGE'] = 'Utilisation mémoire';
$text['MONITORING'] = 'Suivi';
$text['MONITORING_ADMINISTRATION'] = 'Administration du suivi';
$text['NETWORK_TRAFFIC'] = 'Activité réseau';
$text['NEW_HOST'] = 'Nouvelle machine';
$text['PROCESS_COUNT'] = 'Nombre de processus';
$text['SETTINGS'] = 'Paramètres';
$text['SWAP_USAGE'] = "Utilisation de l'échange";
$text['VOLUME_USAGE'] = 'Utilisation des volumes';

?>
