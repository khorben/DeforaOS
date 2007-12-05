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
$text = array();
$text['BROWSER'] = 'Browser';
$text['BROWSER_ADMINISTRATION'] = 'Browser administration';
$text['BYTES'] = 'bytes';
$text['GB'] = 'GB';
$text['GROUP'] = 'Group';
$text['INDEX_OF'] = 'Index of';
$text['KB'] = 'KB';
$text['MB'] = 'MB';
$text['OWNER'] = 'Owner';
$text['SETTINGS'] = 'Settings';
$text['SIZE'] = 'Size';
$text['TB'] = 'TB';
$text['UP_ONE_DIRECTORY'] = 'Up one directory';
global $lang;
if($lang == 'fr')
{
	$text['BROWSER'] = 'Explorateur';
	$text['BROWSER_ADMINISTRATION'] = "Administration de l'explorateur";
	$text['BYTES'] = 'octets';
	$text['GB'] = 'Go';
	$text['GROUP'] = 'Groupe';
	$text['INDEX_OF'] = 'Fichiers de';
	$text['KB'] = 'Ko';
	$text['MB'] = 'Mo';
	$text['OWNER'] = 'Propriétaire';
	$text['SETTINGS'] = 'Paramètres';
	$text['SIZE'] = 'Taille';
	$text['TB'] = 'To';
	$text['UP_ONE_DIRECTORY'] = 'Répertoire parent';
}
_lang($text);


//private
function _get_date($time)
{
	return date('Y-m-d h:m:s', $time);
}

function _get_group($id)
{
	static $cache = array();

	if(isset($cache[$id]))
		return $cache[$id];
	if(($gr = posix_getgrgid($id)) == FALSE)
		$cache[$id] = $id;
	else
		$cache[$id] = $gr['name'];
	return $cache[$id];
}

function _get_size($size)
{
	if($size < 1024)
		return $size.' '.BYTES;
	if(($size = round($size / 1024)) < 1024)
		return $size.' '.KB;
	if(($size = round($size / 1024)) < 1024)
		return $size.' '.MB;
	if(($size = round($size / 1024)) < 1024)
		return $size.' '.GB;
	$size = round($size / 1024);
	return $size.' '.TB;
}

function _get_user($id)
{
	static $cache = array();

	if(isset($cache[$id]))
		return $cache[$id];
	if(($pw = posix_getpwuid($id)) == FALSE)
		$cache[$id] = $id;
	else
		$cache[$id] = $pw['name'];
	return $cache[$id];
}


//public
//browser_admin
function browser_admin($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title browser">'._html_safe(BROWSER_ADMINISTRATION)
			."</h1>\n");
	print('<h2 class="title settings">'._html_safe(SETTINGS)."</h2>\n");
	if(($configs = _config_list('browser')))
	{
		$module = 'browser';
		$action = 'config_update';
		include('./system/config.tpl');
	}
}


//browser_config_update
function browser_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return browser_admin(array());
}


//browser_default
function browser_default($args)
{
	$file = _default_file(isset($args['file']) ? $args['file'] : '');
	if(($root = _config_get('browser', 'root')) == FALSE)
		return _error('Internal error');
	if(($st = stat($root.'/'.$file)) == FALSE)
		return _error('Could not open file');
	if($st['mode'] & 040000 == 040000)
		return _default_dir($root, $file, isset($args['sort'])
				? $args['sort'] : FALSE);
	return _default_display($root, $file);
}

function _default_file($file)
{
	$file = stripslashes($file);
	$filev = explode('/', $file);
	$file = '';
	foreach($filev as $f)
		if($f == '.' || $f == '..')
			continue;
		else
			$file.='/'.$f;
	return '/'.trim($file, '/');
}

function _default_dir(&$root, &$file, $sort)
{
	if(($dir = opendir($root.'/'.$file)) == FALSE)
		return _error('Could not open directory');
	print('<h1 class="title browser">'._html_safe(INDEX_OF.' '.$file)
			."</h1>\n");
	$entries = array();
	require_once('./system/mime.php');
	while(($de = readdir($dir)) != FALSE)
	{
		if(strcmp($de, '..') == 0 || $de[0] == '.')
			continue; /* FIXME latter should be optional */
		$entry = array('module' => 'browser', 'action' => 'default',
				'id' => $file.'/'.$de, 'name' => $de,
				'icon' => 'icons/16x16/mime/default.png',
				'thumbnail' => 'icons/48x48/mime/default.png');
		$entry['id'] = '/'.trim($entry['id'], '/');
		if(($st = lstat($root.'/'.$entry['id'])) == FALSE)
		{
			$entries[] = $entry;
			continue;
		}
		$entry['args'] = 'file='.$entry['id'];
		$entry['uid'] = _get_user($st['uid']);
		$entry['gid'] = _get_group($st['gid']);
		$entry['size'] = $st['size'];
		$entry['dsize'] = _get_size($st['size']);
		$entry['mtime'] = _get_date($st['mtime']);
		if(($st['mode'] & 040000) == 040000)
		{
			$entry['icon'] = 'icons/16x16/mime/folder.png';
			$entry['thumbnail'] = 'icons/48x48/mime/folder.png';
		}
		else
		{
			$mime = _mime_from_ext($de);
			$entry['icon'] = 'icons/16x16/mime/'.$mime.'.png';
			if(!is_readable($entry['icon']))
				$entry['icon'] = 'icons/16x16/mime/default.png';
			$entry['thumbnail'] = 'icons/48x48/mime/'.$mime.'.png';
			if(!is_readable($entry['thumbnail']))
				$entry['thumbnail'] = 'icons/48x48/mime/'
					.'default.png';
		}
		$entries[] = $entry;
	}
	$fsort = $sort;
	switch($sort)
	{
		case 'dsize':	$fsort = 'size';	break;
		case 'gid':				break;
		case 'mtime':				break;
		case 'uid':				break;
		case 'name':				break;
		default:	$fsort = 'name';
				$sort = 'name';		break;
	}
	$func = '_entries_sort_'.$fsort;
	usort($entries, $func);
	$toolbar = array();
	$toolbar[] = array('class' => 'parent_directory',
			'link' => _html_link('browser', '', dirname($file)),
			'title' => UP_ONE_DIRECTORY);
	$class = array('dsize' => SIZE, 'uid' => OWNER, 'gid' => GROUP,
			'mtime' => DATE);
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'view' => 'details', 'entries' => $entries,
				'class' => $class, 'module' => 'browser',
				'action' => 'default', 'sort' => $sort));
}

function _default_display(&$root, &$file)
{
	include('./modules/browser/display.tpl');
}

function _entries_sort_gid(&$a, &$b)
{
	return strcmp($a['gid'], $b['gid']);
}

function _entries_sort_mtime(&$a, &$b)
{
	return strcmp($a['mtime'], $b['mtime']);
}

function _entries_sort_name(&$a, &$b)
{
	return strcmp($a['id'], $b['id']);
}

function _entries_sort_size(&$a, &$b)
{
	return $a['size'] - $b['size'];
}

function _entries_sort_uid(&$a, &$b)
{
	return strcmp($a['uid'], $b['uid']);
}


//browser_download
function browser_download($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return browser_default($args);
}


//browser_system
function browser_system($args)
{
	global $title, $error;

	$title.=' - '.BROWSER;
	if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		if(!isset($args['action']))
			$args['action'] = 'default';
		switch($args['action'])
		{
			case 'admin':
				break;
			case 'download':
				if(!isset($args['file']))
					break;
				$error = _system_download($args['file']);
				break;
		}
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
	{
		switch($args['action'])
		{
			case 'config_update':
				$error = _system_config_update($args);
				break;
		}
	}
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('browser', $args);
	header('Location: '._module_link('browser', 'admin'));
	exit(0);
}

function _system_download($file)
{
	$file = _default_file($file);
	if(($root = _config_get('browser', 'root')) == FALSE)
		return 'Internal error';
	if(($fp = fopen($root.'/'.$file, 'rb')) == FALSE)
		return 'Could not open file';
	if(($st = fstat($fp)) == FALSE || $st['mode'] & 040000 == 040000)
	{
		fclose($fp);
		return 'Is a directory';
	}
	header('Content-Length: '.$st['size']);
	require_once('./system/mime.php');
	$mime = _mime_from_ext(addslashes($file));
	header('Content-Type: '.$mime);
	$file = basename($file);
	if(strpos($file, "\n") === FALSE)
		header('Content-Disposition: attachment; filename="'.$file.'"');
	fpassthru($fp);
	fclose($fp);
	exit(0);
}

?>
