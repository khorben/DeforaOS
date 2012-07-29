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
		return $res[0][0];
	}


	//PgsqlDatabase::isFalse
	public function isFalse($value)
	{
		return $value == 'f';
	}


	//PgsqlDatabase::isTrue
	public function isTrue($value)
	{
		return $value == 't';
	}


	//useful
	//PgsqlDatabase::enum
	public function enum(&$engine, $table, $field)
	{
		$query = $this->query_enum;
		if(($res = $this->query($engine, $query, array(
					'table' => $table,
					'field' => $table.'_'.$field)))
				=== FALSE)
			return array();
		$res = explode("'", $res[0]['constraint']);
		$str = array();
		for($i = 1, $cnt = count($res); $i < $cnt; $i+=2)
			$str[] = $res[$i];
		return $str;
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
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		if(($query = $this->prepare($query, $parameters)) === FALSE)
			return FALSE;
		if($config->getVariable('database', 'debug'))
			$engine->log('LOG_DEBUG', $query);
		$error = FALSE;
		if(($res = pg_query($this->handle, $query)) === FALSE)
		{
			$error = pg_last_error($this->handle);
			if($error !== FALSE)
				$engine->log('LOG_DEBUG', $error);
			return FALSE;
		}
		//FIXME use pg_fetchall() instead (breaks _sql_single() for now)
		for($array = array(); ($a = pg_fetch_array($res)) != FALSE;
				$array[] = $a);
		return $array;
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
		return TRUE;
	}


	//PgsqlDatabase::escape
	protected function escape($string)
	{
		if(is_bool($string))
			$string = $string ? '1' : '0';
		return "'".pg_escape_string($this->handle, $string)."'";
	}


	//private
	//properties
	private $handle = FALSE;

	//queries
	private $query_enum = 'SELECT
		pg_catalog.pg_get_constraintdef(r.oid) AS constraint
		FROM pg_catalog.pg_class c, pg_catalog.pg_constraint r
		WHERE c.oid=r.conrelid AND c.relname=:table
		AND conname=:field';
}

?>
