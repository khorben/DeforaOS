<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
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



//Template
abstract class Template
{
	//public
	//methods
	//static
	//Template::attachDefault
	public static function attachDefault(&$engine)
	{
		global $config;
		$ret = FALSE;
		$priority = 0;

		if(($name = $config->getVariable('template', 'backend'))
				!== FALSE)
		{
			$res = require_once('./templates/'.$name.'.php');
			if($res === FALSE)
				return FALSE;
			$name .= 'Template';
			$ret = new $name();
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' (default)');
			$ret->attach($engine);
			return $ret;
		}
		if(($dir = opendir('templates')) === FALSE)
			return FALSE;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4) != '.php')
				continue;
			require_once('./templates/'.$de);
			$name = substr($de, 0, strlen($de) - 4);
			$name .= 'Template';
			$template = new $name();
			if(($p = $template->match($engine)) <= $priority)
				continue;
			$ret = $template;
			$priority = $p;
		}
		closedir($dir);
		if($ret != FALSE)
		{
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' with priority '.$priority);
			$ret->attach($engine);
		}
		return $ret;
	}


	//virtual
	abstract public function render(&$engine, $content);


	//protected
	//virtual
	abstract protected function match(&$engine);
	abstract protected function attach(&$engine);
}

?>
