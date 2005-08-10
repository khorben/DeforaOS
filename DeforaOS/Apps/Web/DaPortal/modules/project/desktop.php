<?php
//modules/project/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Projects';
$list = 1;
$actions = array('download' => 'Downloads',
		'installer' => 'Installer',
		'list' => 'List',
		'package' => 'Packages',
		'bug_list' => 'Reports');
global $lang;
if($lang == 'fr')
{
	$title = 'Projets';
	$actions['download'] = 'Téléchargement';
	$actions['installer'] = 'Installeur';
	$actions['list'] = 'Liste';
	$actions['package'] = 'Packages';
	$actions['bug_list'] = 'Rapports';
}


?>
