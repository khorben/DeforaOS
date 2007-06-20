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


$title = 'Webmail';
$icon = 'webmail.png';
$admin = 1;
global $user_id;
$list = $user_id != 0 ? 1 : 0;
$actions = array('default' => 'Message list',
		'logout' => 'Logout');
global $lang;
if($lang == 'de')
{
	$actions['logout'] = 'Ausloggen';
}
else if($lang == 'fr')
{
	$actions = array('default' => 'Boite de réception',
			'logout' => 'Déconnexion');
}


?>
