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
//- prepare queries using SQLite3::prepare()



require_once('./system/database.php');


//Sqlite3Database
class Sqlite3Database extends Database
{
	//Sqlite3Database::~Sqlite3Database
	function __destruct()
	{
		if($this->handle == NULL)
			return;
		$this->handle->close();
	}


	//public
	//methods
	//accessors
	//Sqlite3Database::getLastId
	public function getLastId(&$engine, $table, $field)
	{
		if($this->handle === FALSE)
			return FALSE;
		//FIXME return the real last ID for $table_$field
		return $this->handle->lastInsertRowID();
	}


	//useful
	//Sqlite3Database::enum
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


	//Sqlite3Database::offset
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


	//Sqlite3Database::query
	public function query(&$engine, $query, $parameters = FALSE)
	{
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
		if($config->getVariable('database', 'debug'))
			$engine->log('LOG_DEBUG', $query);
		$error = FALSE;
		if(($res = $this->handle->query($query)) === FALSE)
		{
			if($error !== FALSE)
				$engine->log('LOG_DEBUG', $error);
			return FALSE;
		}
		return $res->fetchArray(SQLITE3_BOTH);
	}


	//protected
	//methods
	//Sqlite3Database::match
	protected function match(&$engine)
	{
		global $config;

		if(!class_exists('SQLite3'))
			return 0;
		if($config->getVariable('database::sqlite3', 'filename')
				!== FALSE)
			return 1;
		return 0;
	}


	//Sqlite3Database::attach
	protected function attach(&$engine)
	{
		global $config;

		if(($filename = $config->getVariable('database::sqlite3',
						'filename')) === FALSE)
			return $engine->log('LOG_ERR',
					'Database filename not defined');
		if(($this->handle = new SQLite3($filename)) == NULL)
			return $engine->log('LOG_ERR',
					'Could not open database');
		return TRUE;
	}


	//Sqlite3Database::escape
	protected function escape($string)
	{
		return "'".SQLite3::escapeString($string)."'";
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
