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
	//Engine::getCredentials()
	public function getCredentials()
	{
		if($this->auth === FALSE)
		{
			require_once('./system/auth.php');
			$this->auth = Auth::attachDefault($this);
		}
		if($this->auth === FALSE)
			return new AuthCredentials;
		return $this->auth->getCredentials();
	}


	//Engine::getDatabase()
	public function getDatabase()
	{
		if($this->database === FALSE)
		{
			require_once('./system/database.php');
			$this->database = Database::attachDefault($this);
		}
		return $this->database;
	}


	//Engine::getType
	public function getType()
	{
		return $this->type;
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
		//FIXME use PHP's configured facility instead (error_log())
		$message = $_SERVER['SCRIPT_FILENAME'].": ".$message;
		switch($priority)
		{
			case 'LOG_ALERT':
				syslog(LOG_ALERT, $message);
				break;
			case 'LOG_CRIT':
				syslog(LOG_CRIT, $message);
				break;
			case 'LOG_DEBUG':
				if(Engine::$debug !== TRUE)
					return FALSE;
				syslog(LOG_DEBUG, $message);
				break;
			case 'LOG_EMERG':
				syslog(LOG_EMERG, $message);
				break;
			case 'LOG_ERR':
				syslog(LOG_ERR, $message);
				break;
			case 'LOG_NOTICE':
				syslog(LOG_NOTICE, $message);
				break;
			case 'LOG_WARNING':
				syslog(LOG_WARNING, $message);
				break;
			default:
				syslog(LOG_INFO, $message);
				break;
		}
		return FALSE;
	}


	//Engine::process
	public function process($request)
	{
		if($request === FALSE)
			return FALSE;
		return $request->process($this);
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
		if($ret != FALSE)
		{
			$ret->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' with priority '.$priority);
			$ret->attach();
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
