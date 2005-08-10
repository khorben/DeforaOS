<?php
//system/lang.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

/* FIXME look through the following in order and check if they're known and
 * enabled:
 * - fetch current value in session
 * - fetch default users' value from database if logged in
 * - read through thes one supplied by the navigator */
if(!isset($lang) && ereg('^([a-zA-Z]+)', $_SERVER['HTTP_ACCEPT_LANGUAGE'], $r))
	$lang = $r[1];


function lang($text)
{
	$keys = array_keys($text);
	foreach($keys as $k)
		define($k, $text[$k]);
}


//global translations
$text['_BY_'] = ' by ';
$text['_FOR_'] = ' for ';
$text['ADMINISTRATOR'] = 'Administrator';
$text['AUTHOR'] = 'Author';
$text['CONTENT'] = 'Content';
$text['DATE'] = 'Date';
$text['DESCRIPTION'] = 'Description';
$text['HOMEPAGE'] = 'Homepage';
$text['LOGIN'] = 'Login';
$text['LOGOUT'] = 'Logout';
$text['MEMBERS'] = 'Members';
$text['MESSAGE'] = 'Message';
$text['NAME'] = 'Name';
$text['PASSWORD'] = 'Password';
$text['PERMISSION_DENIED'] = 'Permission denied';
$text['SEND'] = 'Send';
$text['TITLE'] = 'Title';
$text['TYPE'] = 'Type';
$text['UPDATE'] = 'Update';
$text['USERNAME'] = 'Username';
if($lang == 'de')
{
	$text['_BY_'] = ' von ';
	$text['_FOR_'] = ' für ';
	$text['AUTHOR'] = 'Autor';
	$text['DESCRIPTION'] = 'Beschreibung';
	$text['LOGIN'] = 'Einloggen';
	$text['LOGOUT'] = 'Ausloggen';
	$text['MESSAGE'] = 'Mitteilung';
	$text['PASSWORD'] = 'Passwort';
	$text['TITLE'] = 'Titel';
	$text['TYPE'] = 'Typ';
	$text['USERNAME'] = 'Benutzername';
}
else if($lang == 'fr')
{
	$text['_BY_'] = ' par ';
	$text['_FOR_'] = ' pour ';
	$text['ADMINISTRATOR'] = 'Administrateur';
	$text['AUTHOR'] = 'Auteur';
	$text['CONTENT'] = 'Contenu';
	$text['HOMEPAGE'] = 'Accueil';
	$text['LOGIN'] = 'Authentification';
	$text['LOGOUT'] = 'Déconnexion';
	$text['MEMBERS'] = 'Membres';
	$text['NAME'] = 'Nom';
	$text['PASSWORD'] = 'Mot de passe';
	$text['PERMISSION_DENIED'] = 'Permission non accordée';
	$text['SEND'] = 'Envoyer';
	$text['TITLE'] = 'Titre';
	$text['UPDATE'] = 'Mettre à jour';
	$text['USERNAME'] = 'Utilisateur';
}
lang($text);

?>
