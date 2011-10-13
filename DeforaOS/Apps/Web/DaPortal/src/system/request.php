<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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


//Request
class Request
{
	//public
	//methods
	function __construct($engine, $module = FALSE, $action = FALSE,
			$id = FALSE, $title = FALSE, $parameters = FALSE)
	{
		if($module === FALSE
				|| $this->setModule($engine, $module) === FALSE)
			return;
		if($action !== FALSE
				&& $this->setAction($engine, $action) === FALSE)
			return;
		if($id !== FALSE && $this->setId($engine, $id) === FALSE)
			return;
		if($title !== FALSE
				&& $this->setTitle($engine, $title) === FALSE)
			return;
		if($parameters !== FALSE
				&& $this->setParameters($engine, $parameters)
				=== FALSE)
			return;
	}


	//useful
	public function process(&$engine)
	{
		$engine->log('LOG_DEBUG', 'Processing request: module "'
				.$this->module.'"'
				.', action "'.$this->action.'"');
		if($this->handle === FALSE
				|| ($ret = $this->handle->call($engine, $this))
				=== NULL)
			return $engine->log('LOG_ERR', 'Unable to process'
					.' request');
		return $ret;
	}


	//accessors
	public function getAction()
	{
		return $this->action;
	}


	public function getId()
	{
		return $this->id;
	}


	public function getModule()
	{
		return $this->module;
	}


	public function getParameter($name)
	{
		if(!isset($this->parameters[$name]))
			return FALSE;
		return $this->parameters[$name];
	}


	public function getParameters()
	{
		return $this->parameters;
	}


	public function getTitle()
	{
		return $this->title;
	}


	private function setModule(&$engine, $module)
	{
		if(($this->handle = Module::load($engine, $module)) === FALSE)
			return FALSE;
		$this->module = $module;
		return TRUE;
	}


	private function setAction(&$engine, $action)
	{
		if(strchr($action, '.') !== FALSE
				|| strchr($action, '/') !== FALSE)
		{
			$engine->log('LOG_ERR', 'Invalid action '.$action);
			$this->handle = FALSE;
			return FALSE;
		}
		$this->action = basename($action);
		return TRUE;
	}


	private function setId(&$engine, $id)
	{
		if(!is_numeric($id))
		{
			$engine->log('LOG_ERR', 'Invalid ID '.$id);
			$this->handle = FALSE;
			return FALSE;
		}
		$this->id = $id;
		return TRUE;
	}


	private function setParameters(&$engine, $parameters)
	{
		if(!is_array($parameters))
			return FALSE;
		$this->parameters = $parameters;
	}


	public function setTitle(&$engine, $title)
	{
		$this->title = $title;
		return TRUE;
	}


	//private
	//properties
	private $handle = FALSE;
	private $module = FALSE;
	private $action = FALSE;
	private $id = FALSE;
	private $title = FALSE;
	private $parameters = FALSE;
}

?>
