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

$text['APPEARANCE'] = 'Apparence';
$text['CREATE'] = 'Créer';
$text['EMAIL_ALREADY_ASSIGNED'] = 'Cet e-mail est déjà utilisé';
$text['EMAIL_INVALID'] = "Cet e-mail n'est pas valide";
$text['MY_CONTENT'] = 'Mes contenus';
$text['MY_PROFILE'] = 'Mon profil';
$text['NEW_USER'] = 'Nouvel utilisateur';
$text['REGISTER'] = "S'inscrire";
$text['SETTINGS'] = 'Paramètres';
$text['THEME'] = 'Thème';
$text['USER_ALREADY_ASSIGNED'] = 'Cet utilisateur existe déjà';
$text['USER_LOGIN'] = 'Authentification utilisateur';
$text['USER_MODIFICATION'] = "Modification d'utilisateur";
$text['USER_REGISTRATION'] = 'Inscription utilisateur';
$text['USERS'] = 'Utilisateurs';
$text['USERS_ADMINISTRATION'] = 'Administration des utilisateurs';
$text['WRONG_PASSWORD'] = 'Mot de passe incorrect';
$text['YOUR_PASSWORD_IS'] = 'Votre mot de passe est';

?>
