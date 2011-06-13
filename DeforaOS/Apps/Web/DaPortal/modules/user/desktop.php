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


global $lang;
$text = array();
$text['APPEARANCE'] = 'Appearance';
$text['CONTENT'] = 'Content';
$text['LOGIN'] = 'Login';
$text['LOGOUT'] = 'Logout';
$text['PROFILE'] = 'Profile';
$text['USER'] = 'User';
$text['USERS'] = 'Users';
if($lang == 'de')
{
	$text['LOGIN'] = 'Einloggen';
	$text['LOGOUT'] = 'Ausloggen';
	$text['USER'] = 'Benutzer';
	$text['USERS'] = 'Benutzer';
}
else if($lang == 'fr')
{
	$text['APPEARANCE'] = 'Apparence';
	$text['CONTENT'] = 'Contenus';
	$text['LOGIN'] = 'Authentification';
	$text['LOGOUT'] = 'Déconnexion';
	$text['PROFILE'] = 'Profil';
	$text['USER'] = 'Utilisateur';
	$text['USERS'] = 'Utilisateurs';
}
_lang($text);

$title = USERS;
$icon = 'users';
$admin = 1;
$list = 1;
$search = 0;
global $user_id;
if($user_id != 0)
{
	$actions = array('appearance' => APPEARANCE,
			'display' => CONTENT, 'modify' => PROFILE,
			'logout' => LOGOUT);
	$title = USER;
}
else
	$actions = array('login' => LOGIN);

?>
