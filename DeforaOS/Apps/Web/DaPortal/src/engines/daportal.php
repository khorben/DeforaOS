<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require_once('./engines/http.php');


//DaPortalEngine
//XXX the mixed-case name makes it impossible to instantiate automatically!
//    this is however intentional, as this engine is only useful during the
//    migration period to the newer framework.
class DaPortalEngine extends HttpEngine
{
	//public
	//methods
	//essential
	//DaPortalEngine::match
	public function match()
	{
		//never instantiate automatically
		return -1;
	}


	//DaPortalEngine::attach
	public function attach()
	{
		parent::attach();
	}


	//accessors
	//DaPortalEngine::getRequest
	public function getRequest()
	{
		$r = $this->_getRequest();
		//sanitize
		return new Request($this, $this->sanitize($r->getModule()),
				$this->sanitize($r->getAction()),
				$this->sanitize($r->getId()),
				$this->sanitize($r->getTitle()),
				$this->sanitize($r->getParameters()));
	}


	//private
	//DaPortalEngine::_getRequest
	private function _getRequest()
	{
		global $friendlylinks;
		$idempotent = TRUE;

		if($friendlylinks != 1 || !isset($_SERVER['PATH_INFO']))
			return parent::getRequest();
		if(($path = explode('/', $_SERVER['PATH_INFO'])) === FALSE
				|| count($path) < 2)
			return parent::getRequest();
		array_shift($path);
		$module = array_shift($path);
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$parameters = array();
		if(count($path) >= 1)
		{
			$action = array_shift($path);
			if(is_numeric($action))
			{
				$id = $action;
				$action = FALSE;
			}
			else if(count($path) >= 1)
			{
				$id = array_shift($path);
				if(!is_numeric($id))
					$id = FALSE;
			}
			if(count($path) == 1 && is_numeric($id))
				//FIXME validate title? within modules?
				$title = array_shift($path);
		}
		if(count($path) != 0)
			return parent::getRequest();
		if($module !== FALSE)
			$parameters['module'] = $module;
		if($action !== FALSE)
			$parameters['action'] = $action;
		if($id !== FALSE)
			$parameters['id'] = $id;
		if($title !== FALSE)
			$parameters['title'] = $title;
		foreach($_REQUEST as $key => $value)
			if(get_magic_quotes_gpc())
				$parameters[stripslashes($key)]
					= stripslashes($value);
			else
				$parameters[$key] = $value;
		if($_SERVER['REQUEST_METHOD'] == 'POST')
		{
			$var = '_POST';
			$idempotent = FALSE;
		}
		else if($_SERVER['REQUEST_METHOD'] == 'GET')
			$var = '_GET';
		else
			return new Request($this, $module, $action, $id, $title,
					$parameters);
		$keys = array('module', 'action', 'id', 'title');
		foreach($keys as $k)
			if(isset($parameters[$k]))
				$$var[$k] = addslashes($parameters[$k]);
		$ret = new Request($this, $module, $action, $id, $title,
				$parameters);
		$ret->setIdempotent($idempotent);
		return $ret;
	}


	//DaPortalEngine::sanitize
	private function sanitize($arg)
	{
		if($arg === FALSE)
			return FALSE;
		if(is_array($arg))
		{
			$ret = array();
			foreach($arg as $key => $value)
				$ret[addslashes($key)] = addslashes($value);
			return $ret;
		}
		return addslashes($arg);
	}
}

?>
