<?php //modules/admin/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


$title = 'Administration';
$icon = 'admin.png';
$admin = 1;
$list = 0;
global $user_id;
require_once('./system/user.php');
if(!_user_admin($user_id))
	return;
$list = 1;
$actions = array('admin' => array('title' => 'Modules'));

$actions['admin']['actions'] = array();
$modules = _sql_array('SELECT name FROM daportal_module ORDER BY name ASC');
foreach($modules as $m)
{
	if(($d = _module_desktop($m['name'])) == FALSE
			|| $d['admin'] != 1)
		continue;
	unset($d['actions']);
	$d['args'] = '&module='.$d['name'].'&action=admin';
	$actions['admin']['actions'][] = $d;
}

?>
