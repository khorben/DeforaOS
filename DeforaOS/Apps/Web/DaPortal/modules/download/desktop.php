<?php //modules/download/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['DOWNLOADS'] = 'Downloads';
global $lang;
_lang($text);

$title = DOWNLOADS;
$admin = 1;
$list = 1;

?>
