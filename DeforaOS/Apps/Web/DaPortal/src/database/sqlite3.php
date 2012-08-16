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



require_once('./system/database.php');


//SQLite3Database
class SQLite3Database extends Database
{
	//SQLite3Database::~SQLite3Database
	function __destruct()
	{
		if($this->handle == NULL)
			return;
		$this->handle->close();
	}


	//public
	//methods
	//accessors
	//SQLite3Database::getLastId
	public function getLastId($engine, $table, $field)
	{
		if($this->handle === FALSE)
			return FALSE;
		//FIXME return the real last ID for $table_$field
		return $this->handle->lastInsertRowID();
	}


	//useful
	//SQLite3Database::enum
	public function enum($engine, $table, $field)
	{
		$query = 'SELECT name FROM '.$table.'_enum_'.$field;
		if(($res = $this->query($engine, $query)) === FALSE)
			return FALSE;
		$ret = array();
		foreach($res as $r)
			$ret[] = $r['name'];
		return $ret;
	}


	//SQLite3Database::offset
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


	//SQLite3Database::query
	public function query($engine, $query, $parameters = FALSE)
	{
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		if($config->getVariable('database', 'debug'))
			$engine->log('LOG_DEBUG', $query);
		if(($stmt = $this->prepare($query)) === FALSE)
			return $engine->log('LOG_ERR',
					'Could not prepare statement: '
					.$this->handle->lastErrorMsg());
		if($parameters === FALSE)
			$parameters = array();
		if($stmt->clear() !== TRUE)
			return $engine->log('LOG_ERR',
					'Could not clear statement: '
					.$this->handle->lastErrorMsg());
		foreach($parameters as $k => $v)
			$stmt->bindValue(':'.$k, $v);
		if(($res = $stmt->execute()) === FALSE)
			return $engine->log('LOG_ERR',
					'Could not execute statement: '
					.$this->handle->lastErrorMsg());
		//fetch all the results
		for($ret = array();
			($r = $res->fetchArray(SQLITE3_BOTH)) !== FALSE;
			$ret[] = $r);
		return $ret;
	}


	//protected
	//methods
	//SQLite3Database::match
	protected function match($engine)
	{
		global $config;

		if(!class_exists('SQLite3'))
			return 0;
		if($config->getVariable('database::sqlite3', 'filename')
				!== FALSE)
			return 100;
		return 0;
	}


	//SQLite3Database::attach
	protected function attach($engine)
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


	//SQLite3Database::escape
	protected function escape($string)
	{
		return "'".SQLite3::escapeString($string)."'";
	}


	//SQLite3Database::prepare
	public function prepare($query, $parameters = FALSE)
	{
		static $statements = array();

		if(isset($statements[$query]))
			return $statements[$query];
		$statements[$query] = $this->handle->prepare($query);
		return $statements[$query];
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
