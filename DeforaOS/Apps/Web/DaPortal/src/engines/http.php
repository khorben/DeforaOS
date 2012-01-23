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



require_once('./system/engine.php');
require_once('./system/page.php');
require_once('./system/template.php');


//HttpEngine
class HttpEngine extends Engine
{
	//public
	//methods
	//essential
	//HttpEngine::match
	public function match()
	{
		if(!isset($_SERVER['SERVER_PROTOCOL']))
			return -1;
		switch($_SERVER['SERVER_PROTOCOL'])
		{
			case 'HTTP/1.1':
			case 'HTTP/1.0':
				return 100;
			default:
				if(strncmp($_SERVER['SERVER_PROTOCOL'], 'HTTP/',
							5) == 0)
					return 0;
				break;
		}
		return -1;
	}


	//HttpEngine::attach
	public function attach()
	{
		$this->setType('text/html');
		$request = $this->getRequest();
		$this->log('LOG_DEBUG', 'URL is '.$this->getUrl($request));
		if(!isset($_SERVER['SCRIPT_NAME'])
				|| substr($_SERVER['SCRIPT_NAME'], -10)
				!= '/index.php')
		{
			header('Location: '.$parent);
			exit(0);
		}
	}


	//accessors
	//HttpEngine::getRequest
	public function getRequest()
	{
		global $config;

		$request = array();
		$module = FALSE;
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$parameters = FALSE;
		if($_SERVER['REQUEST_METHOD'] == 'GET')
			$request = $_GET;
		else if($_SERVER['REQUEST_METHOD'] == 'POST')
			$request = $_POST;
		foreach($request as $key => $value)
		{
			$k = get_magic_quotes_gpc() ? stripslashes($key) : $key;
			$v = get_magic_quotes_gpc() ? stripslashes($value)
				: $value;
			switch($k)
			{
				case 'module':
				case 'action':
				case 'id':
				case 'title':
					$$k = $request[$key];
					break;
				default:
					if($parameters === FALSE)
						$parameters = array();
					$parameters[$k] = $v;
					break;
			}
		}
		return new Request($this, $module, $action, $id, $title,
				$parameters);
	}


	//HttpEngine::getUrl
	public function getUrl($request)
	{
		if($request === FALSE)
			return FALSE;
		$url = $_SERVER['SERVER_NAME'];
		if(isset($_SERVER['HTTPS']))
		{
			if($_SERVER['SERVER_PORT'] != 443)
				$url .= ':'.$_SERVER['SERVER_PORT'];
			$url = 'https://'.$url;
		}
		else if($_SERVER['SERVER_PORT'] != 80)
			$url = 'http://'.$url.':'.$_SERVER['SERVER_PORT'];
		else
			$url = 'http://'.$url;
		$url .= '/';
		$name = ltrim($_SERVER['SCRIPT_NAME'], '/');
		/* $parent = $uri;
		if(($d = dirname($name)) != '.')
			$parent .= $d; */
		$url .= $name;
		if(($module = $request->getModule()) !== FALSE)
		{
			$url .= '?module='.urlencode($module);
			if(($action = $request->getAction()) !== FALSE)
				$url .= '&action='.urlencode($action);
			if(($id = $request->getId()) !== FALSE)
				$url .= '&id='.urlencode($id);
			if(($title = $request->getTitle()) !== FALSE)
				$url .= '&id='.urlencode($title);
			if(($args = $request->getParameters()) !== FALSE)
				foreach($args as $key => $value)
					$url .= '&'.urlencode($key)
					.'='.urlencode($title);
		}
		return $url;
	}


	//HttpEngine::isIdempotent
	public function isIdempotent($request)
	{
		//FIXME move this code into the Auth module
		$token = $request->getParameter('_token');

		if($token === FALSE || $_SERVER['REQUEST_METHOD'] != 'POST')
			return TRUE;
		if(!@session_start())
			return FALSE;
		if(!isset($_SESSION['tokens']) || !isset($_SESSION['tokens'])
				|| !is_array($_SESSION['tokens'])
				|| !isset($_SESSION['tokens'][$token]))
			return TRUE;
		$time = $_SESSION['tokens'][$token];
		//tokens can only be used once
		unset($_SESSION['tokens'][$token]);
		if(!is_integer($time) || $time < time())
			return FALSE;
		return FALSE;
	}


	//useful
	//HttpEngine::render
	public function render($page)
	{
		global $config;

		$type = $this->getType();
		header('Content-Type: '.$type); //XXX escape
		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			header('Content-Encoding: '.$charset); //XXX escape
		if(($location = $page->getProperty('location')) !== FALSE)
			header('Location: '.$location); //XXX escape
		switch($type)
		{
			case 'text/html':
			default:
				if(($template = Template::attachDefault($this))
						=== FALSE)
					return FALSE;
				if(($page = $template->render($this, $page))
						=== FALSE)
					return FALSE;
				require_once('./system/format.php');
				$output = Format::attachDefault($this, $type);
				$output->render($this, $page);
				break;
		}
	}


	//private
	//properties
	private $uri;
}

?>
