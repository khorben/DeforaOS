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
require_once('./system/locale.php');
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
		Locale::init($this);
		$this->setType('text/html');
		if($this->getDebug())
		{
			$request = $this->getRequest();
			$url = $this->getUrl($request);
			$this->log('LOG_DEBUG', 'URL is '.$url);
		}
		//FIXME this code is wrong (parent is not defined)
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
		$idempotent = TRUE;
		$module = FALSE;
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$parameters = FALSE;
		if($_SERVER['REQUEST_METHOD'] == 'GET')
			$request = $_GET;
		else if($_SERVER['REQUEST_METHOD'] == 'POST')
		{
			$request = $_POST;
			$idempotent = FALSE;
		}
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
		$request = new Request($this, $module, $action, $id, $title,
				$parameters);
		$request->setIdempotent($idempotent);
		return $request;
	}


	//HttpEngine::getUrl
	public function getUrl($request, $absolute = TRUE)
	{
		//FIXME do not include parameters for a POST request
		if($request === FALSE)
			return FALSE;
		$name = ltrim($_SERVER['SCRIPT_NAME'], '/');
		if($absolute)
		{
			$url = $_SERVER['SERVER_NAME'];
			if(isset($_SERVER['HTTPS']))
			{
				if($_SERVER['SERVER_PORT'] != 443)
					$url .= ':'.$_SERVER['SERVER_PORT'];
				$url = 'https://'.$url;
			}
			else if($_SERVER['SERVER_PORT'] != 80)
				$url = 'http://'.$url.':'
					.$_SERVER['SERVER_PORT'];
			else
				$url = 'http://'.$url;
			$url .= '/'.$name;
		}
		else
			$url = basename($name);
		if(($module = $request->getModule()) !== FALSE)
		{
			$url .= '?module='.urlencode($module);
			if(($action = $request->getAction()) !== FALSE)
				$url .= '&action='.urlencode($action);
			if(($id = $request->getId()) !== FALSE)
				$url .= '&id='.urlencode($id);
			if(($title = $request->getTitle()) !== FALSE)
			{
				$title = str_replace(' ', '-', $title);
				$url .= '&title='.urlencode($title);
			}
			if($this->isIdempotent($request)
					&& ($args = $request->getParameters())
					!== FALSE)
				foreach($args as $key => $value)
					$url .= '&'.urlencode($key)
					.'='.urlencode($value);
		}
		return $url;
	}


	//HttpEngine::isIdempotent
	public function isIdempotent($request)
	{
		$auth = $this->getAuth();

		return $auth->isIdempotent($this, $request);
	}


	//useful
	//HttpEngine::render
	public function render($page)
	{
		global $config;

		$type = $this->getType();
		header('Content-Type: '.$type); //XXX escape
		if($page === FALSE)
			$page = new Page;
		if($page instanceof PageElement)
		{
			if(($charset = $config->getVariable('defaults',
					'charset')) !== FALSE)
				//XXX escape
				header('Content-Encoding: '.$charset);
			if(($location = $page->getProperty('location'))
					!== FALSE)
				header('Location: '.$location); //XXX escape
			switch($type)
			{
				case 'text/html':
				default:
					$template = Template::attachDefault(
							$this);
					if($template === FALSE)
						return FALSE;
					if(($page = $template->render($this,
							$page)) === FALSE)
						return FALSE;
					require_once('./system/format.php');
					$output = Format::attachDefault($this,
							$type);
					$output->render($this, $page);
					break;
			}
		}
		else
			//FIXME find a better way for bigger files (callback?)
			print($page);
	}


	//private
	//properties
	private $uri;
}

?>
