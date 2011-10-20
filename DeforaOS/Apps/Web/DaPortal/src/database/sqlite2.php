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



//Sqlite2Database
require_once('./system/database.php');
class Sqlite2Database extends Database
{
	//Sqlite2Database::~Sqlite2Database
	function __destruct()
	{
		if($this->handle === FALSE)
			return;
		sqlite_close($this->handle);
	}


	//public
	//methods
	//useful
	//Sqlite2Database::enum
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


	//Sqlite2Database::offset
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


	//Sqlite2Database::query
	public function query(&$engine, $query, $parameters = FALSE)
	{
		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
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
	//Sqlite2Database::match
	protected function match(&$engine)
	{
		global $config;

		if($config->getVariable('database::sqlite2', 'filename')
				!== FALSE)
			return 1;
		return 0;
	}


	//Sqlite2Database::attach
	protected function attach(&$engine)
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
		//XXX for the DaPortal engine
		define('SQL_TRUE', 1);
		define('SQL_FALSE', 0);
		return TRUE;
	}


	//Sqlite2Database::escape
	protected function escape($string)
	{
		return sqlite_escape_string($string);
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
