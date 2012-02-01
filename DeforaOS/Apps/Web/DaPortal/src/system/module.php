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
	//Module::Module
	protected function __construct($name)
	{
		$this->name = $name;
	}


	//accessors
	//Module::getId
	public static function getId($engine, $module)
	{
		static $ids = array();

		if(isset($ids[$module]))
			return $ids[$module];
		if($engine === FALSE)
			return FALSE;
		$db = $engine->getDatabase();
		$query = Module::$query_id;
		$args = array('name' => $module);
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			$ids[$module] = $engine->log('LOG_DEBUG', 'Module '
					.$module.' is not enabled');
		else
			$ids[$module] = $res[0]['id'];
		return $ids[$module];
	}


	//static
	//useful
	//Module::load
	public static function load(&$engine, $module)
	{
		if($module === FALSE
				|| Module::getId($engine, $module) === FALSE)
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
		return $ret;
	}


	//virtual
	public abstract function call(&$engine, $request);


	//protected
	//properties
	protected $id = FALSE;
	protected $name = FALSE;


	//private
	//properties
	//queries
	static private $query_id = "SELECT module_id AS id
FROM daportal_module
WHERE daportal_module.enabled='1'
AND name=:name";
}

?>
