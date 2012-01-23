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



//Request
class Request
{
	//public
	//methods
	//Request::Request
	function __construct($engine = FALSE, $module = FALSE, $action = FALSE,
			$id = FALSE, $title = FALSE, $parameters = FALSE)
	{
		global $config;

		if($engine === FALSE)
			return;
		//FIXME if $module === FALSE but $id !== FALSE then guess it?
		if($module === FALSE)
		{
			if(($module = $config->getVariable('defaults',
							'module')) === FALSE)
				return;
			$action = $config->getVariable('defaults', 'action');
			$id = $config->getVariable('defaults', 'id');
			$title = FALSE;
			$parameters = FALSE;
		}
		if($this->setModule($engine, $module) === FALSE
				|| $this->setAction($engine, $action) === FALSE
				|| $this->setId($engine, $id) === FALSE
				|| $this->setTitle($engine, $title) === FALSE
				|| $this->setParameters($engine, $parameters)
				=== FALSE)
			return;
	}


	//accessors
	//Request::getAction
	public function getAction()
	{
		return $this->action;
	}


	//Request::getId
	public function getId()
	{
		return $this->id;
	}


	//Request::getModule
	public function getModule()
	{
		return $this->module;
	}


	//Request::getParameter
	public function getParameter($name)
	{
		if(!isset($this->parameters[$name]))
			return FALSE;
		return $this->parameters[$name];
	}


	//Request::getParameters
	public function getParameters()
	{
		return $this->parameters;
	}


	//Request::getTitle
	public function getTitle()
	{
		return $this->title;
	}


	//private
	//methods
	//accessors
	//Request::setModule
	private function setModule(&$engine, $module)
	{
		if($module === FALSE)
		{
			$this->module = $module;
			return TRUE;
		}
		if(strchr($module, '.') !== FALSE
				|| strchr($module, '/') !== FALSE)
		{
			$engine->log('LOG_DEBUG', 'Invalid module '.$module);
			return $this->reset();
		}
		$this->module = $module;
		return TRUE;
	}


	//Request::setAction
	private function setAction(&$engine, $action)
	{
		if($action === FALSE)
		{
			$this->action = FALSE;
			return TRUE;
		}
		if(strchr($action, '.') !== FALSE
				|| strchr($action, '/') !== FALSE)
		{
			$engine->log('LOG_DEBUG', 'Invalid action '.$action);
			return $this->reset();
		}
		$this->action = basename($action);
		return TRUE;
	}


	//Request::setId
	private function setId(&$engine, $id)
	{
		if($id !== FALSE && !is_numeric($id))
		{
			$engine->log('LOG_DEBUG', 'Invalid ID '.$id);
			return $this->reset();
		}
		$this->id = $id;
		return TRUE;
	}


	//Request::setParameters
	private function setParameters(&$engine, $parameters)
	{
		if($parameters !== FALSE && !is_array($parameters))
			return $this->reset();
		$this->parameters = $parameters;
		return TRUE;
	}


	//Request::setTitle
	private function setTitle(&$engine, $title)
	{
		$this->title = $title;
		return TRUE;
	}


	//useful
	//Request::reset
	private function reset()
	{
		$this->module = FALSE;
		$this->action = FALSE;
		$this->id = FALSE;
		$this->title = FALSE;
		$this->parameters = FALSE;
		return FALSE;
	}


	//private
	//properties
	private $module = FALSE;
	private $action = FALSE;
	private $id = FALSE;
	private $title = FALSE;
	private $parameters = FALSE;
}

?>
