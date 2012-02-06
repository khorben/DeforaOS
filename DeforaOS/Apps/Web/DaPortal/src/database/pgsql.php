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


//PgsqlDatabase
class PgsqlDatabase extends Database
{
	//PgsqlDatabase::~PgsqlDatabase
	function __destruct()
	{
		if($this->handle === FALSE)
			return;
		pg_close($this->handle);
	}


	//public
	//methods
	//accessors
	//PgsqlDatabase::getLastId
	public function getLastId(&$engine, $table, $field)
	{
		if($this->handle === FALSE)
			return FALSE;
		//FIXME untested
		$query = "SELECT currval('".$table."_".$field."_seq');";
		if(($res = $this->query($engine, $query)) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}


	//useful
	//PgsqlDatabase::enum
	public function enum(&$engine, $table, $field)
	{
		//FIXME implement
		return FALSE;
	}


	//PgsqlDatabase::offset
	public function offset($limit, $offset = FALSE)
	{
		$ret = '';

		//FIXME untested
		//XXX report errors
		if($offset !== FALSE && is_numeric($offset))
			$ret .= " OFFSET $offset";
		if(!is_numeric($limit))
			$limit = 0;
		$ret .= " LIMIT $limit";
		return $ret;
	}


	//PgsqlDatabase::query
	public function query(&$engine, $query, $parameters = FALSE)
	{
		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
		$engine->log('LOG_DEBUG', $query);
		$error = FALSE;
		if(($ret = pg_query($this->handle, $query, SQLITE_BOTH,
						$error)) === FALSE)
		{
			if($error !== FALSE)
				$engine->log('LOG_DEBUG', $error);
			return FALSE;
		}
		return pg_fetch_all($ret);
	}


	//protected
	//methods
	//PgsqlDatabase::match
	protected function match(&$engine)
	{
		global $config;

		//we probably cannot detect a PostgreSQL database
		return 0;
	}


	//PgsqlDatabase::attach
	protected function attach(&$engine)
	{
		global $config;
		$str = '';
		$sep = '';
		$array = array('username' => 'user', 'password' => 'password',
				'database' => 'dbname', 'hostname' => 'host',
				'port' => 'port',
				'timeout' => 'connect_timeout',
				'service' => 'service');

		foreach($array as $k => $v)
			if(($p = $config->getVariable('database::pgsql', $k))
					!== FALSE)
			{
				$str .= $sep.$v."='$p'"; //XXX escape?
				$sep = ' ';
			}
		if(($this->handle = pg_connect($str)) === FALSE)
			return $engine->log('LOG_ERR',
					'Could not open database');
		//XXX for the DaPortal engine
		define('SQL_TRUE', 1);
		define('SQL_FALSE', 0);
		return TRUE;
	}


	//PgsqlDatabase::escape
	protected function escape($string)
	{
		return "'".pg_escape_string($this->handle, $string)."'";
	}


	//private
	//properties
	private $handle = FALSE;
}

?>
