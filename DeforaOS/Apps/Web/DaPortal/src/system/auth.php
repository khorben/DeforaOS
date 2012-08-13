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


//AuthCredentials
class AuthCredentials
{
	//public
	//essential
	public function __construct($uid = FALSE, $username = FALSE,
			$gid = FALSE, $admin = FALSE)
	{
		if($uid === FALSE || !is_string($username) || !is_bool($admin))
			return;
		$this->setUserId($uid, $admin);
		if($gid === FALSE)
			return;
		$this->setGroupId($gid);
		$this->username = $username;
	}


	//accessors
	//AuthCredentials::getGroupId
	public function getGroupId()
	{
		return $this->gid;
	}


	//AuthCredentials::getUserId
	public function getUserId()
	{
		return $this->uid;
	}


	//AuthCredentials::isAdmin
	public function isAdmin()
	{
		return $this->admin;
	}


	//AuthCredentials::setGroupId
	public function setGroupId($gid)
	{
		if(!is_numeric($gid))
			return FALSE;
		$this->gid = $gid;
		return TRUE;
	}


	//AuthCredentials::setUserId
	public function setUserId($uid, $admin = FALSE)
	{
		if(!is_numeric($uid) || !is_bool($admin))
		{
			$this->uid = 0;
			$this->admin = FALSE;
			return FALSE;
		}
		$this->uid = $uid;
		$this->admin = $admin;
		return TRUE;
	}


	//private
	//properties
	private $uid = 0;
	private $username = FALSE;
	private $gid = 0;
	private $group = FALSE;
	private $groups = array();
	private $admin = FALSE;
}


//Auth
abstract class Auth
{
	//public
	//accessors
	//Auth::getCredentials
	public function getCredentials(&$engine)
	{
		if($this->credentials === FALSE)
			$this->credentials = new AuthCredentials;
		return $this->credentials;
	}


	//Auth::getVariable
	public function getVariable(&$engine, $variable)
	{
		//FIXME implement (through the database?)
		return FALSE;
	}


	//Auth::setIdempotent
	public function setIdempotent(&$engine, $request, $idempotent)
	{
		$request->setIdempotent($idempotent);
	}


	//Auth::setCredentials
	public function setCredentials(&$engine, $credentials)
	{
		$this->credentials = $credentials;
		return TRUE;
	}


	//Auth::setVariable
	public function setVariable(&$engine, $variable, $value)
	{
		//FIXME implement (through the database?)
		return FALSE;
	}


	//static
	//Auth::attachDefault
	public static function attachDefault(&$engine)
	{
		global $config;
		$ret = FALSE;
		$priority = 0;

		if(($name = $config->getVariable('auth', 'backend')) !== FALSE)
		{
			$res = require_once('./auth/'.$name.'.php');
			if($res === FALSE)
				return FALSE;
			$name .= 'Auth';
			$ret = new $name();
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' (default)');
			$ret->attach($engine);
			return $ret;
		}
		if(($dir = opendir('auth')) === FALSE)
			return FALSE;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4) != '.php')
				continue;
			require_once('./auth/'.$de);
			$name = substr($de, 0, strlen($de) - 4);
			$name .= 'Auth';
			$auth = new $name();
			if(($p = $auth->match($engine)) <= $priority)
				continue;
			$ret = $auth;
			$priority = $p;
		}
		closedir($dir);
		if($ret !== FALSE)
		{
			$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret)
					.' with priority '.$priority);
			$ret->attach($engine);
		}
		return $ret;
	}


	//protected
	//properties
	protected $credentials = FALSE;


	//methods
	//virtual
	abstract protected function match(&$engine);
	abstract protected function attach(&$engine);
}

?>
