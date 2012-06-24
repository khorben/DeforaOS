<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



//Mime
class Mime
{
	//public
	//methods
	//static
	//Mime::getIcon
	static public function getIcon(&$engine, $filename, $size = 48)
	{
		//FIXME no longer hardcode the icon theme
		$default = 'icons/gnome/gnome-icon-theme/'.$size.'x'.$size
			.'/mimetypes/gtk-file.png';

		if(Mime::init($engine) === FALSE)
			return $default;
		if(($type = Mime::getType($engine, $filename, FALSE)) === FALSE)
			return $default;
		//substitutions
		//FIXME there are many more substitutions to apply
		$icon = str_replace('application/', 'application-', $type);
		$icon = 'icons/gnome/gnome-icon-theme/'.$size.'x'.$size
			.'/mimetypes/gnome-mime-'.$icon.'.png';
		return $icon;
	}


	//Mime::getType
	static public function getType(&$engine, $filename,
			$default = 'application/octet-stream')
	{
		if(Mime::init($engine) === FALSE)
			return $default;
		//FIXME use lstat() if the filename is absolute or relative
		foreach(Mime::$types as $g)
			if(isset($g[1]) && fnmatch($g[1], $filename))
				return $g[0];
		return $default;
	}


	//private
	//static
	//properties
	static private $types = FALSE;


	//methods
	//Mime::init
	static private function init(&$engine)
	{
		global $config;

		if(Mime::$types !== FALSE)
			return;
		Mime::$types = array();
		if(($globs = $config->getVariable('mime', 'globs')) === FALSE)
		{
			$engine->log('LOG_WARNING',
					'The globs file is not defined');
			return FALSE;
		}
		if(($globs = file_get_contents($globs)) === FALSE)
		{
			$engine->log('LOG_WARNING',
					'Could not read the globs file');
			return FALSE;
		}
		$globs = explode("\n", $globs);
		foreach($globs as $line)
		{
			if(strlen($line) >= 1 && $line[0] == '#')
				continue;
			else
				Mime::$types[] = explode(':', $line);
		}
		return TRUE;
	}
}

?>
