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
	public function getLastId($engine, $table, $field)
	{
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		//determine the underlying backend
		if(($backend = $config->getVariable('database::pdo', 'dsn'))
				!== FALSE)
		{
			$backend = explode(':', $backend);
			if(is_array($backend))
				$backend = $backend[0];
		}
		switch($backend)
		{
			case 'pgsql':
				//PostgreSQL requires a sequence object
				$seq = $table.'_'.$field.'_seq';
				return $this->handle->lastInsertId($seq);
			default:
				return $this->handle->lastInsertId();
		}
	}


	//useful
	//PdoDatabase::enum
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


	//PdoDatabase::prepare
	public function prepare($query, $parameters = FALSE)
	{
		static $statements = array();

		if(isset($statements[$query]))
			return $statements[$query];
		$statements[$query] = $this->handle->prepare($query);
		return $statements[$query];
	}


	//PdoDatabase::query
	public function query($engine, $query, $parameters = FALSE)
	{
		global $config;

		if($this->handle === FALSE)
			return FALSE;
		if($config->getVariable('database', 'debug'))
			$engine->log('LOG_DEBUG', $query);
		if(($stmt = $this->prepare($query)) === FALSE)
		{
			$error = $this->handle->errorInfo();
			$error[] = '';
			$error[] = 'Unknown error';
			return $engine->log('LOG_ERR',
				'Could not prepare statement: '
				.$error[0].': '.$error[2]);
		}
		if($parameters === FALSE)
			$parameters = array();
		$args = array();
		foreach($parameters as $k => $v)
			$args[':'.$k] = $v;
		if($stmt->execute($args) !== TRUE)
		{
			$error = $stmt->errorInfo();
			$error[] = '';
			$error[] = 'Unknown error';
			return $engine->log('LOG_ERR',
					'Could not execute query: '
					.$error[0].': '.$error[2]);
		}
		return $stmt->fetchAll();
	}


	//PdoDatabase::transactionBegin
	public function transactionBegin($engine)
	{
		if($this->handle === FALSE)
			return FALSE;
		if($this->transaction++ == 0)
			return $this->handle->beginTransaction();
		return TRUE;
	}


	//PdoDatabase::transactionCommit
	public function transactionCommit($engine)
	{
		if($this->handle === FALSE)
			return FALSE;
		if($this->transaction == 0)
			return FALSE;
		if($this->transaction-- == 1)
			return $this->handle->commit();
		return TRUE;
	}


	//PdoDatabase::transactionRollback
	public function transactionRollback($engine)
	{
		if($this->handle === FALSE)
			return FALSE;
		if($this->transaction == 0)
			return FALSE;
		if($this->transaction-- == 1)
			return $this->handle->rollback();
		return TRUE;
	}


	//protected
	//methods
	//PdoDatabase::match
	protected function match($engine)
	{
		global $config;

		if(!class_exists('PDO'))
			return 0;
		if($config->getVariable('database::pdo', 'dsn') !== FALSE)
			return 100;
		return 0;
	}


	//PdoDatabase::attach
	protected function attach($engine)
	{
		global $config;

		if(($dsn = $config->getVariable('database::pdo', 'dsn'))
				=== FALSE)
			return $engine->log('LOG_ERR',
					'Data Source Name (DSN) not defined');
		try {
			$this->handle = new PDO($dsn);
		} catch(PDOException $e) {
			$message = 'Could not open database: '.$e->getMessage();
			return $engine->log('LOG_ERR', $message);
		}
		return TRUE;
	}


	//PdoDatabase::escape
	protected function escape($string)
	{
		if($this->handle === FALSE)
			return FALSE;
		if(is_bool($string))
			return $string ? "'1'" : "'0'";
		return $this->handle->quote($string);
	}


	//private
	//properties
	private $handle = FALSE;
	private $transaction = 0;
}

?>
