<?php //modules/project/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Projects';
$admin = 1;
$list = 1;
$actions = array('download' => 'Downloads',
		'installer' => 'Installer',
		'list' => 'List',
		'package' => 'Packages',
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
	$actions['installer'] = 'Installeur';
	$actions['list'] = 'Liste';
	$actions['bug_list'] = 'Rapports';
}


?>
