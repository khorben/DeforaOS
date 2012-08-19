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



require_once('./system/database.php');


//SQLite2Database
class SQLite2Database extends Database
{
	//SQLite2Database::~SQLite2Database
	function __destruct()
	{
		if($this->handle === FALSE)
			return;
		sqlite_close($this->handle);
	}


	//public
	//methods
	//accessors
	//SQLite2Database::getLastId
	public function getLastId($engine, $table, $field)
	{
		if($this->handle === FALSE)
			return FALSE;
		//FIXME return the real last ID for $table_$field
		return sqlite_last_insert_rowid($this->handle);
	}


	//useful
	//SQLite2Database::enum
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


	//SQLite2Database::query
	public function query($engine, $query, $parameters = FALSE)
	{
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
		if($config->getVariable('database', 'debug'))
			$engine->log('LOG_DEBUG', $query);
		$error = FALSE;
		if(($ret = sqlite_query($this->handle, $query, SQLITE_BOTH,
						$error)) === FALSE)
		{
			if($error !== FALSE)
				$engine->log('LOG_DEBUG', $error);
			return FALSE;
		}
		return sqlite_fetch_all($ret);
	}


	//protected
	//methods
	//SQLite2Database::match
	protected function match($engine)
	{
		global $config;

		if($config->getVariable('database::sqlite2', 'filename')
				!== FALSE)
			return 100;
		return 0;
	}


	//SQLite2Database::attach
	protected function attach($engine)
	{
		global $config;

		if(($filename = $config->getVariable('database::sqlite2',
						'filename')) === FALSE)
			return $engine->log('LOG_ERR',
					'Database filename not defined');
		if(($this->handle = sqlite_open($filename, 0666, $error))
				=== FALSE)
			return $engine->log('LOG_ERR',
					'Could not open database: '.$error);
		return TRUE;
	}


	//SQLite2Database::escape
	protected function escape($string)
	{
		if(is_bool($string))
			return $string ? "'1'" : "'0'";
		return "'".sqlite_escape_string($string)."'";
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
