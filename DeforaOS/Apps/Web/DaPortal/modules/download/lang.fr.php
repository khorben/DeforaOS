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
$text['BACK'] = 'Pr�c�dent';
$text['BROWSE'] = 'Parcourir';
$text['COMMENT'] = 'Commentaire';
$text['CREATE'] = 'Cr�er';
$text['DOWNLOADS_ADMINISTRATION'] = 'Administration des t�l�chargements';
$text['DOWNLOADS_LIST'] = 'Liste des t�l�chargements';
$text['FILE'] = 'Fichier';
$text['FORWARD'] = 'Suivant';
$text['IMAGE_PREVIEW'] = "Aper�u de l'image";
$text['NEW_DIRECTORY'] = 'Nouveau r�pertoire';
$text['OWNER'] = 'Propri�taire';
$text['PARENT_DIRECTORY'] = 'R�pertoire parent';
$text['SETTINGS'] = 'Param�tres';

?>
