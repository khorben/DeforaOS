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



require('common.php');
require('config.php');

define(TREE_LEVEL, 1);


function list_dirs($path)
{
	if(($dir = @opendir($path)) == FALSE)
		return FALSE;
	$dirs = array();
	readdir($dir);
	readdir($dir);
	while(($de = readdir($dir)))
	{
		if(!@is_dir($path.'/'.$de))
			continue;
		$dirs[] = $de;
	}
	usort($dirs, strcmp);
	return $dirs;
}


function dir_has_subdirs($path)
{
	if(($dir = @opendir($path)) == FALSE)
		return FALSE;
	readdir($dir);
	readdir($dir);
	while(($de = readdir($dir)))
	{
		if(@is_dir($path.'/'.$de))
			return TRUE;
	}
	return FALSE;
}


function _tree_level($level, $curroot)
{
	global $root;

	$dirs = list_dirs($root.'/'.$curroot);
	if($dirs == FALSE || count($dirs) == 0)
		return;
	$hidden = $level > TREE_LEVEL;
	while($name = array_shift($dirs))
	{
		$path = $curroot.'/'.$name;
		$class = count($dirs) ? 'node' : 'lastnode';
		$collapse = 0;
		$expand = 0;
		if($level == TREE_LEVEL && !is_link($root.'/'.$path)
				&& dir_has_subdirs($root.'/'.$path))
		{
			$expand = 1;
			$img = 'icons/tree/plastnode.gif';
		}
		else if(!is_link($root.'/'.$path)
				&& dir_has_subdirs($root.'/'.$path))
		{
			$collapse = 1;
			$img = 'icons/tree/mlastnode.gif';
		}
		else
			$img = 'icons/tree/'.$class.'.gif';
		include('tree.tpl');
		if(!is_link($root.'/'.$path))
			_tree_level($level+1, $path);
		print("</div>\n");
	}
}


function tree()
{
	_tree_level(1, '/');
}

?>
<html>
	<head>
		<title>Tree</title>
		<link type="text/css" rel="stylesheet" href="explorer.css"/>
		<script type="text/javascript" src="explorer.js"></script>
	</head>
	<body>
		<div class="tree">
<div class="lastnode"><img class="node" <? if(TREE_LEVEL) { ?>src="icons/tree/mlastnode.gif" alt="" onClick="folder_collapse(event)"<? } else { ?>src="icons/tree/plastnode.gif" alt="" onClick="folder_expand(event)"<? } ?>/><a href="explorer.php?folder=/" target="explorer"><img src="icons/16x16/mime/folder.png" alt="directory"/> /</a>
<? tree(); ?>
</div>
		</div>
	</body>
</html>
