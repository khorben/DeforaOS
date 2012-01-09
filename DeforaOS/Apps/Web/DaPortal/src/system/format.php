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



//Format
abstract class Format
{
	//public
	//methods
	//static
	//Format::attachDefault
	public static function attachDefault(&$engine, $type = FALSE)
	{
		global $config;
		$ret = FALSE;
		$priority = 0;

		$name = FALSE;
		if($type !== FALSE)
			$name = $config->getVariable("format::$type",
					'backend');
		if($name === FALSE)
			$name = $config->getVariable('format', 'backend');
		if($name !== FALSE)
		{
			$res = require_once('./formats/'.$name.'.php');
			if($res === FALSE)
				return FALSE;
			$name = ucfirst($name).'Format';
			$ret = new $name();
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' (default)');
			$ret->attach($engine, $type);
			return $ret;
		}
		if(($dir = opendir('formats')) === FALSE)
			return FALSE;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4) != '.php')
				continue;
			require_once('./formats/'.$de);
			$name = substr($de, 0, strlen($de) - 4);
			$name = ucfirst($name).'Format';
			$format = new $name();
			if(($p = $format->match($engine, $type)) <= $priority)
				continue;
			$ret = $format;
			$priority = $p;
		}
		closedir($dir);
		if($ret != FALSE)
		{
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' with priority '.$priority);
			$ret->attach($engine, $type);
		}
		return $ret;
	}


	//protected
	//methods
	//virtual
	abstract protected function match(&$engine, $type);
	abstract protected function attach(&$engine, $type);

	abstract protected function render(&$engine, $page, $filename = FALSE);
}

?>