<?php
//Copyright 2005 Pierre Pronchery
//Some parts Copyright 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//PHP Explorer is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
//PHP Explorer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PHP Explorer; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require('config.php');
require('common.php');


function explorer_download($filename)
{
	global $root, $hidden;

	$filename = $root.'/'.$filename;
	if(!is_readable($filename) || is_dir($filename))
		return include('404.tpl');
	if(!$hidden)
	{
		$f = basename($filename);
		if($f[0] == '.')
			return include('403.tpl');
	}
	$mime = mime_from_ext($filename);
	$client_mime = explode(',', $_SERVER['HTTP_ACCEPT']);
	for($i = 0; $i < count($client_mime); $i++)
	{
		if(($pos = strpos($client_mime[$i], ';')) == FALSE)
			continue;
		$client_mime[$i] = substr($client_mime[$i], 0, $pos);
	}
	$attachment = in_array($mime, $client_mime) ? 'inline' : 'attachment';
	header('Content-Type: '.$mime);
	header('Content-Length: '.filesize($filename));
	header('Content-Disposition: '.$attachment.'; filename="'
			.html_safe(basename($filename)).'"');
	readfile($filename);
}


function _sort_owner($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	$ownera = posix_getpwuid($stata['uid']);
	$ownera = $ownera['name'];
	$ownerb = posix_getpwuid($statb['uid']);
	$ownerb = $ownerb['name'];
	return strcmp($ownera, $ownerb);
}

function _sort_group($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	$groupa = posix_getgrgid($stata['gid']);
	$groupa = $ownera['name'];
	$groupb = posix_getgrgid($statb['gid']);
	$groupb = $ownerb['name'];
	return strcmp($groupa, $groupb);
}

function _sort_size($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	return $stata['size'] < $statb['size'];
}

function _sort_date($a, $b)
{
	global $root, $path;

	$stata = lstat($root.'/'.$path.'/'.$a);
	$statb = lstat($root.'/'.$path.'/'.$b);
	return $stata['date'] < $statb['date'];
}

function explorer_folder($folder, $sort)
{
	global $root, $hidden, $path;

	$path = $folder;
	if(($dir = opendir($root.'/'.$folder)) == FALSE)
		return;
	readdir($dir);
	readdir($dir);
	$files = array();
	if(!$hidden)
		while($de = readdir($dir))
		{
			if($de[0] != '.')
				$files[] = $de;
		}
	else
		while($de = readdir($dir))
			$files[] = $de;
	switch($sort)
	{
		case 'owner':
			usort($files, _sort_owner);
			break;
		case 'group':
			usort($files, _sort_group);
			break;
		case 'size':
			usort($files, _sort_size);
			break;
		case 'date':
			usort($files, _sort_date);
			break;
		case 'name':
		default:
			sort($files);
			break;
	}
	while(($name = array_shift($files)))
	{
		if(@is_dir($root.'/'.$folder.'/'.$name))
		{
			$mime = 'folder';
			$icon = 'icons/16x16/mime/folder.png';
			$thumbnail = 'icons/48x48/mime/folder.png';
			$link = 'explorer.php?folder='.$folder.'/'.$name;
		}
		else
		{
			$mime = mime_from_ext($name);
			$icon = 'icons/16x16/mime/'.$mime.'.png';
			if(!is_readable($icon))
				$icon = 'icons/16x16/mime/default.png';
			$thumbnail = 'icons/48x48/mime/'.$mime.'.png';
			if(!is_readable($thumbnail))
				$thumbnail = 'icons/48x48/mime/default.png';
			$link = 'explorer.php?download='.$folder.'/'.$name;
		}
		$owner = '?';
		$group = '?';
		$size = '?';
		$date = '?';
		if(($stat = @lstat($root.'/'.$folder.'/'.$name)))
		{
			$owner = posix_getpwuid($stat['uid']);
			$owner = $owner['name'];
			$group = posix_getgrgid($stat['gid']);
			$group = $group['name'];
			$size = round($stat['size']/1024);
			$size = $size > 1024 ? round($size/1024).'M'
					: $size.'K';
			$date = date('Y-m-d h:m:s', $stat['mtime']);
		}
		include('entry.tpl');
	}
}


function explorer_sort($folder, $name, $sort)
{
	echo '<div class="'.$name.'">';
	echo '<a href="explorer.php?folder='.html_safe($folder).'&sort='.$name
			.'">'.ucfirst($name).'</a>';
	if($sort == $name || ($sort == '' && $name == 'name'))
		echo ' <img src="icons/16x16/down.png" alt=""/>';
	echo '</div>'."\n";
}


if(isset($_GET['download']))
	return explorer_download(filename_safe($_GET['download']));
$folder = strlen($_GET['folder']) ? filename_safe($_GET['folder']) : '/';
$sort = $_GET['sort'];
include('explorer.tpl');
