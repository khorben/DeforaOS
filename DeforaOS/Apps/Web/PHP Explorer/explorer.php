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
	global $root;

	$filename = $root.'/'.$filename;
	if(!is_readable($filename) | is_dir($filename))
		return include('404.tpl');
	$mime = mime_from_ext($filename);
	header('Content-Type: '.$mime);
	header('Content-Length: '.filesize($filename));
	header('Content-Disposition: attachment; filename="'.html_safe(basename($filename)).'"');
	readfile($filename);
}


function explorer_folder($folder)
{
	global $root;

	if(($dir = opendir($root.'/'.$folder)) == FALSE)
		return;
	readdir($dir);
	readdir($dir);
	$files = array();
	while(($de = readdir($dir)))
		$files[] = $de;
	usort($files, strcmp);
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


if(isset($_GET['download']))
	return explorer_download(filename_safe($_GET['download']));
$folder = strlen($_GET['folder']) ? filename_safe($_GET['folder']) : '/';
include('explorer.tpl');
