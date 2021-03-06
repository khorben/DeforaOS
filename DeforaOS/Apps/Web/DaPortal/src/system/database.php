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



//Database
abstract class Database
{
	//public
	//methods
	//accessors
	//Database::isFalse
	public function isFalse($value)
	{
		return $value == 0;
	}


	//Database::isTrue
	public function isTrue($value)
	{
		return $value == 1;
	}


	//useful
	//Database::formatDate
	public function formatDate($engine, $date, $outformat = FALSE,
			$informat = FALSE)
	{
		if($informat === FALSE)
			$informat = '%Y-%m-%d %H:%M:%S';
		if(($tm = strptime($date, $informat)) === FALSE)
			return $date; //XXX better suggestions welcome
		$timestamp = gmmktime($tm['tm_hour'], $tm['tm_min'],
			$tm['tm_sec'], $tm['tm_mon'] + 1, $tm['tm_mday'],
			$tm['tm_year'] + 1900);
		if($outformat === FALSE)
			$outformat = '%d/%m/%Y %H:%M:%S';
		return strftime($outformat, $timestamp);
	}


	//Database::offset
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


	//Database::transactionBegin
	public function transactionBegin($engine)
	{
		return $this->query($engine, 'BEGIN');
	}


	//Database::transactionCommit
	public function transactionCommit($engine)
	{
		return $this->query($engine, 'COMMIT');
	}


	//Database::transactionRollback
	public function transactionRollback($engine)
	{
		return $this->query($engine, 'ROLLBACK');
	}


	//static
	//Database::attachDefault
	public static function attachDefault($engine)
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
			$name .= 'Database';
			$ret = new $name();
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' (default)');
			if($ret->attach($engine) === FALSE)
				return FALSE;
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
			$name .= 'Database';
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
			if($ret->attach($engine) === FALSE)
				return FALSE;
		}
		return $ret;
	}


	//virtual
	abstract public function getLastId($engine, $table, $field);

	abstract public function enum($engine, $table, $field);
	abstract public function query($engine, $query, $parameters = FALSE);


	//protected
	//methods
	//virtual
	abstract protected function match($engine);
	abstract protected function attach($engine);

	abstract protected function escape($string);


	//useful
	//Database::prepare
	protected function prepare($query, $parameters = FALSE)
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
			$to[] = ($v !== NULL) ? $this->escape($v) : 'NULL';
		}
		//FIXME should really use preg_replace() with proper matching
		return str_replace($from, $to, $query);
	}
}


//DatabaseDummy
class DatabaseDummy extends Database
{
	//public
	//methods
	//accessors
	//DatabaseDummy::getLastId
	public function getLastId($engine, $table, $field)
	{
		//always fail
		return FALSE;
	}


	//useful
	//DatabaseDummy::enum
	public function enum($engine, $table, $field)
	{
		//always fail
		return FALSE;
	}


	//DatabaseDummy::query
	public function query($engine, $query, $parameters = FALSE)
	{
		//always fail
		return FALSE;
	}


	//protected
	//methods
	//essential
	//DatabaseDummy::match
	protected function match($engine)
	{
		//never match
		return 0;
	}


	//DatabaseDummy::attach
	protected function attach($engine)
	{
		//always succeed
		return TRUE;
	}


	//useful
	//DatabaseDummy::escape
	protected function escape($string)
	{
		return str_replace("'", "''", $string);
	}
}

?>
