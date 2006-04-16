<?php
//system/lang.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _lang($text)
{
	$keys = array_keys($text);
	foreach($keys as $k)
		define($k, $text[$k]);
}


function _lang_check($lang)
{
	return _sql_single('SELECT enabled FROM daportal_lang'
			." WHERE enabled='1' AND lang_id='$lang';");
}


if(isset($_POST['lang']) && _lang_check($_POST['lang']))
{
	$lang = $_POST['lang'];
	$_SESSION['lang'] = $lang;
}
else if(isset($_SESSION['lang']) && _lang_check($_SESSION['lang']))
	$lang = $_SESSION['lang'];
else
{
	for($hal = $_SERVER['HTTP_ACCEPT_LANGUAGE'];
			ereg('^,?([a-zA-Z]+)(;q=[0-9.]+)?(.*)$', $hal,
					$regs);
			$hal = $regs[3])
	{
		if(!_lang_check($regs[1]))
			continue;
		$lang = $regs[1];
		break;
	}
}
if(!isset($lang) && !($lang = _config_get('admin', 'lang'))
		&& !_lang_check($lang))
	$lang = 'en';
$locale = $lang.'_'.strtoupper($lang);
if(!setlocale(LC_ALL, $locale.'@euro', $locale, $lang))
	_warning('Unable to set locale');
//lang
$text = array();
$text['_BY_'] = ' by ';
$text['_FOR_'] = ' for ';
$text['ABOUT'] = 'About';
$text['ADMINISTRATOR'] = 'Administrator';
$text['AUTHOR'] = 'Author';
$text['CONTENT'] = 'Content';
$text['DATE'] = 'Date';
$text['DATE_FORMAT'] = '%A, %B %e %Y, %H:%M';
$text['DELETE'] = 'Delete';
$text['DESCRIPTION'] = 'Description';
$text['DISABLE'] = 'Disable';
$text['DISABLED'] = 'Disabled';
$text['EDIT'] = 'Edit';
$text['ENABLE'] = 'Enable';
$text['ENABLED'] = 'Enabled';
$text['FILTER'] = 'Filter';
$text['HOMEPAGE'] = 'Homepage';
$text['INVALID_ARGUMENT'] = 'Invalid argument';
$text['LOGIN'] = 'Login';
$text['LOGOUT'] = 'Logout';
$text['MEMBERS'] = 'Members';
$text['MESSAGE'] = 'Message';
$text['MODIFY'] = 'Modify';
$text['NAME'] = 'Name';
$text['NEWS'] = 'News';
$text['NO'] = 'No';
$text['PASSWORD'] = 'Password';
$text['PERMISSION_DENIED'] = 'Permission denied';
$text['REPLY'] = 'Reply';
$text['SEARCH'] = 'Search';
$text['SEND'] = 'Send';
$text['TITLE'] = 'Title';
$text['TYPE'] = 'Type';
$text['UPDATE'] = 'Update';
$text['USERNAME'] = 'Username';
$text['YES'] = 'Yes';
if($lang == 'de')
{
	$text['_BY_'] = ' von ';
	$text['_FOR_'] = ' für ';
	$text['AUTHOR'] = 'Autor';
	$text['DATE_FORMAT'] = '%A %e %B %Y, %H:%M';
	$text['DESCRIPTION'] = 'Beschreibung';
	$text['LOGIN'] = 'Einloggen';
	$text['LOGOUT'] = 'Ausloggen';
	$text['MESSAGE'] = 'Mitteilung';
	$text['NO'] = 'Nein';
	$text['PASSWORD'] = 'Passwort';
	$text['SEARCH'] = 'Suche';
	$text['TITLE'] = 'Titel';
	$text['TYPE'] = 'Typ';
	$text['USERNAME'] = 'Benutzername';
	$text['YES'] = 'Ja';
}
else if($lang == 'fr')
{
	$text['_BY_'] = ' par ';
	$text['_FOR_'] = ' pour ';
	$text['ABOUT'] = 'A propos';
	$text['ADMINISTRATOR'] = 'Administrateur';
	$text['AUTHOR'] = 'Auteur';
	$text['CONTENT'] = 'Contenu';
	$text['DATE_FORMAT'] = '%A %e %B %Y, %H:%M';
	$text['DELETE'] = 'Supprimer';
	$text['DISABLE'] = 'Désactiver';
	$text['DISABLED'] = 'Désactivé';
	$text['EDIT'] = 'Modifier';
	$text['ENABLE'] = 'Activer';
	$text['ENABLED'] = 'Activé';
	$text['FILTER'] = 'Filtrer';
	$text['HOMEPAGE'] = 'Accueil';
	$text['INVALID_ARGUMENT'] = 'Argument invalide';
	$text['LOGIN'] = 'Authentification';
	$text['LOGOUT'] = 'Déconnexion';
	$text['MEMBERS'] = 'Membres';
	$text['MODIFY'] = 'Modifier';
	$text['NAME'] = 'Nom';
	$text['NEWS'] = 'Actualités';
	$text['NO'] = 'Non';
	$text['PASSWORD'] = 'Mot de passe';
	$text['PERMISSION_DENIED'] = 'Permission non accordée';
	$text['REPLY'] = 'Répondre';
	$text['SEARCH'] = 'Recherche';
	$text['SEND'] = 'Envoyer';
	$text['TITLE'] = 'Titre';
	$text['UPDATE'] = 'Mettre à jour';
	$text['USERNAME'] = 'Utilisateur';
	$text['YES'] = 'Oui';
}
_lang($text);

?>
