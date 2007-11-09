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
$modules = _sql_array('SELECT name FROM daportal_module'
		." WHERE enabled='1' ORDER BY name ASC");
foreach($modules as $m)
{
	if(($d = _module_desktop($m['name'])) == FALSE
			|| $d['admin'] != 1)
		continue;
	unset($d['actions']);
	$d['args'] = 'module='.$d['name'].'&action=admin';
	$actions['admin']['actions'][] = $d;
}

?>
