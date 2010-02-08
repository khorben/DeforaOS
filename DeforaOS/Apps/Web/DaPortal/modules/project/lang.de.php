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

$text['BROWSE_SOURCE'] = 'Quellcode betrachten';
$text['DESCRIPTION'] = 'Beschreibung';
$text['NEW_PROJECT'] = 'Neues projekt';
$text['PRIORITY'] = 'Priorität';
$text['PROJECT'] = 'Projekt';
$text['PROJECT_LIST'] = 'Projektliste';
$text['PROJECTS'] = 'Projekte';
$text['QUELLCODE'] = 'Quellcode';
$text['STATE'] = 'Stand';
$text['TIMELINE'] = 'Fortschritt';

?>
