<?php //modules/project/desktop.php


$title = 'Projects';
$icon = 'project.png';
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

$user = array(array('icon' => 'project.png', 'name' => $title),
		array('icon' => 'bug.png',
			'name' => $actions['bug_list'],
			'action' => 'bug_list'));

?>
