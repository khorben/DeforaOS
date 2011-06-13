<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



function _icon($name, $size = 16)
{
	static $cache = array();
	$id = $name.'_'.$size;

	if(isset($cache[$id]))
		return $cache[$id];
	$cache[$id] = _icon_find($name, $size);
	return $cache[$id];
}

function _icon_find($name, $size)
{
	global $icontheme;
	$root = $_SERVER['DOCUMENT_ROOT'].'/'.dirname($_SERVER['SCRIPT_NAME'])
		.'/';
	$path = './icons/'.$icontheme.'/'.$icontheme.'.php';
	$prefix = FALSE;
	$icon = array();

	if(is_readable($path))
		include($path);
	if($path !== FALSE && isset($icon[$name]))
	{
		$prefix = ($prefix !== FALSE) ? '/'.$prefix : '';
		$path = 'icons/'.$icontheme.$prefix.'/'.$size.'x'.$size
			.'/'.$icon[$name].'.png';
		if(is_readable($root.'/'.$path))
			return $path;
	}
	$path = $size.'x'.$size.'/'.$name.'.png';
	if(is_readable($root.'/icons/'.$icontheme.'/'.$path))
		return 'icons/'.$icontheme.'/'.$path;
	if(is_readable($root.'/icons/'.$path))
		return 'icons/'.$path;
	return 'icons/'.$size.'x'.$size.'/default.png';
}

?>
