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


$title = 'Projects';
$icon = 'project';
$admin = 1;
$list = 1;
$search = 1;
$actions = array('list' => 'List',
		'bug_list' => 'Reports');
global $lang;
if($lang == 'de')
{
	$title = 'Projekte';
	$actions['list'] = 'Projektliste';
}
else if($lang == 'fr')
{
	$title = 'Projets';
	$actions['download'] = 'Téléchargement';
	$actions['list'] = 'Liste';
	$actions['bug_list'] = 'Rapports';
}

$user = array(array('icon' => 'project', 'name' => $title),
		array('icon' => 'bug',
			'name' => $actions['bug_list'],
			'action' => 'bug_list'));

?>
