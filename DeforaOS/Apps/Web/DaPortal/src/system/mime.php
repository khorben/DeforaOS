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
	static public function get(&$engine, $filename,
			$default = 'application/octet-stream')
	{
		if(Mime::init($engine) === FALSE)
			return $default;
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