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



require_once('./system/module.php');
require_once('./system/request.php');


//Engine
abstract class Engine
{
	//public
	//virtual
	//essential
	abstract public function match();
	abstract public function attach();

	//accessors
	abstract public function getRequest();

	//useful
	abstract public function render($content);


	//methods
	//accessors
	//Engine::getCredentials
	public function getCredentials()
	{
		if($this->auth === FALSE)
		{
			require_once('./system/auth.php');
			$this->auth = Auth::attachDefault($this);
		}
		if($this->auth === FALSE)
			return new AuthCredentials;
		return $this->auth->getCredentials($this);
	}


	//Engine::getDatabase
	public function getDatabase()
	{
		if($this->database === FALSE)
		{
			require_once('./system/database.php');
			$this->database = Database::attachDefault($this);
		}
		return $this->database;
	}


	//Engine::getDebug
	public function getDebug()
	{
		return Engine::$debug;
	}


	//Engine::getType
	public function getType()
	{
		return $this->type;
	}


	//Engine::getUrl
	public function getUrl($request, $absolute = TRUE)
	{
		return FALSE;
	}


	//Engine::isIdempotent
	public function isIdempotent($request)
	{
		return TRUE;
	}


	//Engine::setCredentials
	public function setCredentials($cred)
	{
		if($this->auth === FALSE)
		{
			require_once('./system/auth.php');
			$this->auth = Auth::attachDefault($this);
		}
		if($this->auth === FALSE)
			return FALSE;
		return $this->auth->setCredentials($cred);
	}


	//Engine::setDebug
	public function setDebug($debug)
	{
		$this->debug = ($debug !== FALSE) ? TRUE : FALSE;
	}


	//Engine::setType
	protected function setType($type)
	{
		$this->type = $type;
	}


	//useful
	//Engine::log
	public function log($priority, $message)
	{
		if(Engine::$debug !== TRUE)
			return FALSE;
		switch($priority)
		{
			case 'LOG_ALERT':
			case 'LOG_CRIT':
			case 'LOG_EMERG':
				$level = 'Alert';
				break;
			case 'LOG_DEBUG':
				$level = 'Debug';
				break;
			case 'LOG_ERR':
				$level = 'Error';
				break;
			case 'LOG_WARNING':
				$level = 'Warning';
				break;
			case 'LOG_INFO':
			case 'LOG_NOTICE':
			default:
				$level = 'Info';
				break;
		}
		$message = $_SERVER['SCRIPT_FILENAME'].": $level: $message";
		error_log($message, 0);
		return FALSE;
	}


	//Engine::process
	public function process($request)
	{
		if($request === FALSE
				|| ($module = $request->getModule()) === FALSE)
			return $this->log('LOG_ERR', 'Unable to process empty'
					.' request');
		$action = $request->getAction();
		$this->log('LOG_DEBUG', "Processing request: module $module"
				.(($action !== FALSE) ? ", action $action"
					: ''));
		if(($handle = Module::load($this, $module)) === FALSE)
			return FALSE;
		return $handle->call($this, $request);
	}


	//static
	//useful
	//Engine::attachDefault
	public static function attachDefault()
	{
		global $config;
		$ret = FALSE;
		$priority = 0;

		if($config->getVariable(FALSE, 'debug') == '1')
			Engine::$debug = TRUE;
		if(($name = $config->getVariable('engine', 'backend'))
				!== FALSE)
		{
			$res = require_once('./engines/'.$name.'.php');
			if($res === FALSE)
				return FALSE;
			$name = ucfirst($name).'Engine';
			$ret = new $name();
			$ret->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' (default)');
			$ret->attach();
			return $ret;
		}
		if(($dir = opendir('engines')) === FALSE)
			return FALSE;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4) != '.php')
				continue;
			$res = require_once('./engines/'.$de);
			if($res === FALSE)
				continue;
			$name = substr($de, 0, strlen($de) - 4);
			$name = ucfirst($name).'Engine';
			$engine = new $name();
			if(($p = $engine->match()) <= $priority)
				continue;
			$ret = $engine;
			$priority = $p;
		}
		closedir($dir);
		if($ret !== FALSE)
		{
			$ret->attach();
			$ret->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' with priority '.$priority);
		}
		return $ret;
	}


	//protected
	//properties
	protected static $debug = FALSE;


	//private
	//properties
	private $type = FALSE;
	private $auth = FALSE;
	private $database = FALSE;
}

?>
