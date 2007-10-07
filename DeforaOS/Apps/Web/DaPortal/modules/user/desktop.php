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


$title = 'Users';
$icon = 'users.png';
$admin = 1;
$list = 1;
global $user_id;
if($user_id != 0)
{
	$actions = array('appearance' => 'Appearance',
			'display' => 'My Content', 'admin' => 'My Profile',
			'logout' => 'Logout');
	require_once('./system/user.php');
	$title = _user_admin($user_id) ? 'Users' : "User's page";
}
else
	$actions = array('login' => 'Login');

global $lang;
if($lang == 'de')
{
	$title = 'Benutzer';
	if(isset($actions['login']))
		$actions['login'] = 'Einloggen';
	if(isset($actions['logout']))
		$actions['logout'] = 'Ausloggen';
}
else if($lang == 'fr')
{
	$title = 'Utilisateurs';
	if(isset($actions['login']))
		$actions['login'] = 'Authentification';
	if(isset($actions['logout']))
		$actions['logout'] = 'Déconnexion';
}

?>
