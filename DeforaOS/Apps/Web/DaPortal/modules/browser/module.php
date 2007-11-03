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
$text['GROUP'] = 'Group';
$text['INDEX_OF'] = 'Index of';
$text['OWNER'] = 'Owner';
$text['SIZE'] = 'Size';
$text['UP_ONE_DIRECTORY'] = 'Up one directory';
global $lang;
if($lang == 'fr')
{
	$text['BROWSER'] = 'Explorateur';
	$text['GROUP'] = 'Groupe';
	$text['INDEX_OF'] = 'Fichiers de';
	$text['OWNER'] = 'Propriétaire';
	$text['SIZE'] = 'Taille';
	$text['UP_ONE_DIRECTORY'] = 'Répertoire parent';
}
_lang($text);


//private
function _get_user($id)
{
	static $cache = array();

	if(isset($cache[$id]))
		return $cache[$id];
	$cache[$id] = posix_getpwuid($id);
	return $cache[$id];
}

function _get_group($id)
{
	static $cache = array();

	if(isset($cache[$id]))
		return $cache[$id];
	$cache[$id] = posix_getgrgid($id);
	return $cache[$id];
}

function _get_size($size)
{
	if($size < 1024)
		return $size.' bytes';
	if(($size = round($size / 1024)) < 1024)
		return $size.' KB';
	if(($size = round($size / 1024)) < 1024)
		return $size.' MB';
	if(($size = round($size / 1024)) < 1024)
		return $size.' GB';
	$size = round($size / 1024);
	return $size.' TB';
}


//public
//browser_admin
function browser_admin($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title browser">'.BROWSER_ADMINISTRATION.'</h1>'."\n");
}


//browser_default
function browser_default($args)
{
	$file = _default_file(isset($args['id']) ? $args['id'] : '');
	$root = _config_get('browser', 'root');
	if(($st = stat($root.'/'.$file)) == FALSE)
		return _error('Could not open file');
	if($st['mode'] & 040000 == 040000)
		return _default_dir($root, $file);
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

function _default_dir(&$root, &$file)
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
		$user = _get_user($st['uid']);
		$entry['uid'] = isset($user['name']) ? $user['name']
			: $st['uid'];
		$group = _get_group($st['gid']);
		$entry['gid'] = isset($group['name']) ? $group['name']
			: $st['gid'];
		$entry['size'] = _get_size($st['size']);
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
	usort($entries, '_entries_sort');
	$toolbar = array();
	$toolbar[] = array('class' => 'parent_directory',
			'link' => _html_link('browser', '', dirname($file)),
			'title' => UP_ONE_DIRECTORY);
	$class = array('uid' => OWNER, 'gid' => GROUP, 'size' => SIZE);
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $entries, 'class' => $class));
}

function _default_display(&$root, &$file)
{
	include('./modules/browser/display.tpl');
}

function _entries_sort(&$a, &$b)
{
	return strcmp($a['id'], $b['id']);
}


//browser_system
function browser_system($args)
{
	global $title;

	$title.=' - '.BROWSER;
	if(!isset($args['action']))
		$args['action'] = 'default';
	switch($args['action'])
	{
		case 'admin':
			break;
		case 'default':
			if(isset($args['download']) && $args['download'] == 1
					&& isset($args['id']))
				_system_download($args['id']);
			break;
	}
}

function _system_download($id)
{
	$file = _default_file($id);
	if(($root = _config_get('browser', 'root')) == FALSE)
		return 1;
	if(($fp = fopen($root.'/'.$file, 'rb')) == FALSE)
		return 1;
	if(($st = fstat($fp)) == FALSE || $st['mode'] & 040000 == 040000)
	{
		fclose($fp);
		return 1;
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
