<?php //system/lang.de.php

//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

$text['_BY_'] = ' von ';
$text['_FOR_'] = ' für ';
$text['AUTHOR'] = 'Autor';
$text['CONTENT'] = 'Inhalt';
$text['DATE_FORMAT'] = '%A %e %B %Y, %H:%M';
$text['DESCRIPTION'] = 'Beschreibung';
$text['EDIT'] = 'Andern';
$text['LOGIN'] = 'Einloggen';
$text['LOGOUT'] = 'Ausloggen';
$text['MESSAGE'] = 'Mitteilung';
$text['NO'] = 'Nein';
$text['PASSWORD'] = 'Passwort';
$text['READ'] = 'Lesen';
$text['REPLY'] = 'Antworten';
$text['SEARCH'] = 'Suche';
$text['SEND'] = 'Schicken';
$text['TITLE'] = 'Titel';
$text['TYPE'] = 'Typ';
$text['UPDATE'] = 'Anwenden';
$text['USERNAME'] = 'Benutzername';
$text['YES'] = 'Ja';

?>
