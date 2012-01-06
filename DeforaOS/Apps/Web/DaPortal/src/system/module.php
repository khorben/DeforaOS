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



//Module
abstract class Module
{
	//public
	//methods
	//essential
	public function __construct($name)
	{
		$this->name = $name;
	}


	//accessors
	public function getId($engine)
	{
		if($this->id !== FALSE)
			return $this->id;
		$db = $engine->getDatabase();
		$query = $this->query_id;
		$args = array('name' => $this->name);
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
		{
			$engine->log('LOG_DEBUG', 'Module '.$this->name
					.' is not enabled');
			return FALSE;
		}
		return $res[0]['id'];
	}


	//static
	//useful
	public static function load(&$engine, $module)
	{
		if($module === FALSE)
			return FALSE;
		$name = ucfirst($module).'Module';
		if(!class_exists($name))
		{
			$engine->log('LOG_DEBUG', 'Loading module '.$module);
			if(strchr($module, '_') !== FALSE
					|| strchr($module, '.') !== FALSE
					|| strchr($module, '/') !== FALSE)
				return $engine->log('LOG_DEBUG',
						'Invalid module '.$module);
			$path = './modules/'.$module.'/module.php';
			if(!is_readable($path))
				return $engine->log('LOG_ERR',
						'Unreadable module '.$module);
			$res = include_once($path);
			if($res === FALSE)
				return $engine->log('LOG_DEBUG',
						'Unknown module '.$module);
			if(!class_exists($name))
				return $engine->log('LOG_ERR',
						'Undefined module '.$module);
		}
		$ret = new $name($module);
		if($ret->getId($engine) === FALSE)
			return FALSE;
		return $ret;
	}


	//virtual
	public abstract function call(&$engine, $request);


	//private
	//properties
	private $id = FALSE;
	private $name = FALSE;

	//queries
	private $query_id = "SELECT module_id AS id
FROM daportal_module
WHERE daportal_module.enabled='1'
AND name=:name";
}
