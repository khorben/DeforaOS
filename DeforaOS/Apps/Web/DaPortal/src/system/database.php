<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//Database
abstract class Database
{
	//public
	//properties
	static public $true = '1';
	static public $false = '0';


	//methods
	//useful
	public function prepare($query, $parameters = FALSE)
	{
		if($parameters === FALSE)
			$parameters = array();
		if(!is_array($parameters))
			return FALSE;
		$from = array();
		$to = array();
		foreach($parameters as $k => $v)
		{
			$from[] = ':'.$k;
			$to[] = "'".$this->escape($v)."'";
		}
		return str_replace($from, $to, $query);
	}


	//static
	public static function attachDefault(&$engine)
	{
		global $config;
		$ret = FALSE;
		$priority = 0;

		if(($name = $config->getVariable('database', 'backend'))
				!== FALSE)
		{
			$res = require_once('./database/'.$name.'.php');
			if($res === FALSE)
				return FALSE;
			$name = ucfirst($name).'Database';
			$ret = new $name();
			$ret->attach($engine);
			return $ret;
		}
		if(($dir = opendir('database')) === FALSE)
			return FALSE;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4) != '.php')
				continue;
			require_once('./database/'.$de);
			$name = substr($de, 0, strlen($de) - 4);
			$name = ucfirst($name).'Database';
			$db = new $name();
			if(($p = $db->match($engine)) <= $priority)
				continue;
			$ret = $db;
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
	abstract public function enum(&$engine, $table, $field);
	abstract public function offset($limit, $offset = FALSE);
	abstract public function query(&$engine, $query, $parameters = FALSE);


	//protected
	//virtual
	abstract protected function match(&$engine);
	abstract protected function attach(&$engine);

	abstract protected function escape($string);
}

?>