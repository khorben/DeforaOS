<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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
class DaPortalEngine extends HttpEngine
{
	//public
	//methods
	//accessors
	//DaPortalEngine::getRequest
	public function getRequest()
	{
		global $friendlylinks;

		if($friendlylinks != 1
				|| !isset($_SERVER['PATH_INFO']))
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
		{
			//FIXME prepare all SQL queries
			//if(get_magic_quotes_gpc() != 0)
			//	$value = stripslashes($value);
			$parameters[$key] = $value;
		}
		if($_SERVER['REQUEST_METHOD'] == 'POST')
		{
			$_POST['module'] = $module;
			$_POST['action'] = $action;
			$_POST['id'] = $id;
			$_POST['title'] = $title;
		}
		else if($_SERVER['REQUEST_METHOD'] == 'GET')
		{
			$_GET['module'] = $module;
			$_GET['action'] = $action;
			$_GET['id'] = $id;
			$_GET['title'] = $title;
		}
		return new Request($this, $module, $action, $id, $title,
				$parameters);
	}


	//useful
	//DaPortalEngine::attach
	public function attach()
	{
		if(!get_magic_quotes_gpc())
			exit(_error('Magic quotes must be enabled'));
		parent::attach();
	}


	//DaPortalEngine::log
	public function log($priority, $message)
	{
		global $debug;

		if($debug)
			parent::log($priority, $message);
		return FALSE;
	}
}

?>
