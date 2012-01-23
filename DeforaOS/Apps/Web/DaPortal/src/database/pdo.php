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
//FIXME:
//- prepare queries using PDO::prepare()



require_once('./system/database.php');


//PdoDatabase
class PdoDatabase extends Database
{
	//PdoDatabase::~PdoDatabase
	function __destruct()
	{
		$this->handle = FALSE;
	}


	//public
	//methods
	//accessors
	//PdoDatabase::getLastId
	public function getLastId(&$engine, $table, $field)
	{
		if($this->handle === FALSE)
			return FALSE;
		//FIXME some backends require a parameter here
		return $this->handle->lastInsertId();
	}


	//useful
	//PdoDatabase::enum
	public function enum(&$engine, $table, $field)
	{
		$query = 'SELECT name FROM '.$table.'_enum_'.$field;
		if(($res = $this->query($engine, $query)) === FALSE)
			return FALSE;
		$ret = array();
		foreach($res as $r)
			$ret[] = $r['name'];
		return $ret;
	}


	//PdoDatabase::offset
	public function offset($limit, $offset = FALSE)
	{
		//XXX report errors
		if(!is_numeric($limit))
			$limit = 0;
		$ret = " LIMIT $limit";
		if($offset !== FALSE && is_numeric($offset))
			$ret .= " OFFSET $offset";
		return $ret;
	}


	//PdoDatabase::query
	public function query(&$engine, $query, $parameters = FALSE)
	{
		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
		$engine->log('LOG_DEBUG', $query);
		$error = FALSE;
		if(($ret = $this->handle->query($query)) === FALSE)
		{
			if($error !== FALSE)
				$engine->log('LOG_DEBUG', $error);
			return FALSE;
		}
		return $ret->fetchAll();
	}


	//protected
	//methods
	//PdoDatabase::match
	protected function match(&$engine)
	{
		global $config;

		if(!class_exists('PDO'))
			return 0;
		if($config->getVariable('database::pdo', 'dsn') !== FALSE)
			return 1;
		return 0;
	}


	//PdoDatabase::attach
	protected function attach(&$engine)
	{
		global $config;

		if(($dsn = $config->getVariable('database::pdo', 'dsn'))
				=== FALSE)
			return $engine->log('LOG_ERR',
					'Data Source Name (DSN) not defined');
		try {
			$this->handle = new PDO($dsn);
		} catch(PDOException $e) {
			return $engine->log('LOG_ERR',
					'Could not open database: '
					.$e->getMessage());
		}
		//XXX for the DaPortal engine
		define('SQL_TRUE', 1);
		define('SQL_FALSE', 0);
		return TRUE;
	}


	//PdoDatabase::escape
	protected function escape($string)
	{
		if($this->handle === FALSE)
			return FALSE;
		return $this->handle->quote($string);
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
