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



require_once('./system/auth.php');
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
	//Engine::getAuth
	public function getAuth()
	{
		return ($this->_attachAuth() !== FALSE) ? $this->auth : FALSE;
	}


	//Engine::getCredentials
	public function getCredentials()
	{
		if($this->_attachAuth() === FALSE)
			return new AuthCredentials;
		return $this->auth->getCredentials($this);
	}


	//Engine::getDatabase
	public function getDatabase()
	{
		if($this->database === FALSE)
		{
			require_once('./system/database.php');
			if(($this->database = Database::attachDefault($this))
					=== FALSE)
				$this->database = new DatabaseDummy;
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


	//Engine::setCredentials
	public function setCredentials($cred)
	{
		if($this->_attachAuth() === FALSE)
			return FALSE;
		return $this->auth->setCredentials($this, $cred);
	}


	//Engine::setDebug
	public function setDebug($debug)
	{
		$this->debug = ($debug !== FALSE) ? TRUE : FALSE;
	}


	//Engine::setType
	public function setType($type)
	{
		$this->type = $type;
	}


	//useful
	//Engine::log
	public function log($priority, $message)
	{
		switch($priority)
		{
			case 'LOG_ALERT':
			case 'LOG_CRIT':
			case 'LOG_EMERG':
				$level = 'Alert';
				break;
			case 'LOG_DEBUG':
				if(Engine::$debug !== TRUE)
					return FALSE;
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
				if(Engine::$debug !== TRUE)
					return FALSE;
				$level = 'Info';
				break;
		}
		if(!is_string($message))
		{
			ob_start();
			var_dump($message); //XXX potentially multi-line
			$message = ob_get_contents();
			ob_end_clean();
		}
		$message = $_SERVER['SCRIPT_FILENAME'].": $level: $message";
		error_log($message, 0);
		return FALSE;
	}


	//Engine::process
	public function process($request, $internal = FALSE)
	{
		if($request === FALSE
				|| ($module = $request->getModule()) === FALSE)
			return FALSE;
		$action = $request->getAction();
		$this->log('LOG_DEBUG', 'Processing'
				.($internal ? ' internal' : '')
				." request: module $module"
				.(($action !== FALSE) ? ", action $action"
					: ''));
		if(($handle = Module::load($this, $module)) === FALSE)
			return FALSE;
		return $handle->call($this, $request, $internal);
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
			$name .= 'Engine';
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
			$name .= 'Engine';
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


	//methods
	private function _attachAuth()
	{
		if($this->auth !== FALSE)
			return TRUE;
		require_once('./system/auth.php');
		$this->auth = Auth::attachDefault($this);
		return ($this->auth !== FALSE) ? TRUE : FALSE;
	}
}

?>
