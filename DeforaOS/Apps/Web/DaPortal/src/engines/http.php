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
require_once('./system/format.php');
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
		$request = $this->getRequest();
		$url = $this->getUrl($request);

		Locale::init($this);
		$this->setType('text/html');
		if($this->getDebug())
			$this->log('LOG_DEBUG', 'URL is '.$url);
		if(isset($_SERVER['SCRIPT_NAME'])
				&& substr($_SERVER['SCRIPT_NAME'], -10)
				!= '/index.php')
		{
			//FIXME might be an invalid address
	 		header('Location: '.dirname($url));
			exit(0);
		}
	}


	//accessors
	//HttpEngine::getRequest
	public function getRequest()
	{
		global $config;

		if(($private = $config->getVariable('engine::http', 'private'))
				== 1)
			return $this->_getRequestPrivate();
		return $this->_getRequestDo();
	}

	protected function _getRequestDo()
	{
		global $config;

		//XXX hack to avoid testing twice for idempotence
		if($this->request !== FALSE)
			return $this->request;
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
		//collect the parameters
		foreach($request as $key => $value)
		{
			$k = get_magic_quotes_gpc() ? stripslashes($key) : $key;
			//FIXME is this sufficient when not a string? (uploads)
			$v = (is_string($value) && get_magic_quotes_gpc())
				? stripslashes($value) : $value;
			switch($k)
			{
				case '_module':
					$module = $request[$key];
					break;
				case '_action':
					$action = $request[$key];
					break;
				case '_id':
					$id = $request[$key];
					break;
				case '_title':
					$title = $request[$key];
					break;
				default:
					if($parameters === FALSE)
						$parameters = array();
					$parameters[$k] = $v;
					break;
			}
		}
		$this->request = new Request($module, $action, $id, $title,
			$parameters);
		$auth = $this->getAuth();
		$auth->setIdempotent($this, $this->request, $idempotent);
		return $this->request;
	}

	protected function _getRequestPrivate()
	{
		global $config;
		$cred = $this->getCredentials();
		$module = 'user';
		$actions = array('login');

		if(($m = $config->getVariable('engine::http',
				'private::module')) !== FALSE)
			$module = $m;
		if(($a = $config->getVariable('engine::http',
		       		'private::actions')) !== FALSE)
			$actions = explode(',', $a);
		$request = $this->_getRequestDo();
		if($cred->getUserId() == 0)
			if($request->getModule() != $module
					|| !in_array($request->getAction(),
						$actions))
				return new Request($module, $actions[0]);
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
			$url .= '?_module='.urlencode($module);
			if(($action = $request->getAction()) !== FALSE)
				$url .= '&_action='.urlencode($action);
			if(($id = $request->getId()) !== FALSE)
				$url .= '&_id='.urlencode($id);
			if(($title = $request->getTitle()) !== FALSE)
			{
				$title = str_replace(' ', '-', $title);
				$url .= '&_title='.urlencode($title);
			}
			if($request->isIdempotent()
					&& ($args = $request->getParameters())
					!== FALSE)
				foreach($args as $key => $value)
					$url .= '&'.urlencode($key)
						.'='.urlencode($value);
		}
		return $url;
	}


	//useful
	//HttpEngine::render
	public function render($page)
	{
		global $config;
		//XXX escape the headers
		$type = $this->getType();
		$header = 'Content-Type: '.$type;
		$charset = $config->getVariable('defaults', 'charset');

		if($charset !== FALSE)
			$header .= '; charset='.$charset;
		header($header);
		if($page instanceof PageElement)
			return $this->renderPage($page, $type);
		else if(is_resource($page)
				&& get_resource_type($page) == 'stream')
			return $this->renderStream($page, $type);
		else if(is_string($page))
			return $this->renderString($page, $type);
		//default
		return $this->renderPage($page, $type);
	}

	private function renderPage($page, $type)
	{
		if($page !== FALSE)
		{
			if(($location = $page->getProperty('location'))
					!== FALSE)
			header('Location: '.$location); //XXX escape
		}
		switch($type)
		{
			case 'application/rss+xml':
			case 'application/xml':
			case 'text/xml':
				break;
			case 'text/html':
			default:
				$template = Template::attachDefault(
					$this);
				if($template === FALSE)
					return FALSE;
				if(($page = $template->render($this,
					$page)) === FALSE)
					return FALSE;
				break;
		}
		$output = Format::attachDefault($this, $type);
		$output->render($this, $page);
	}

	private function renderStream($fp, $type)
	{
		$disposition = (strncmp('image/', $type, 6) == 0)
			? 'inline' : 'attachment';
		//FIXME also set the filename
		header('Content-Disposition: '.$disposition);
		if(($st = fstat($fp)) !== FALSE)
		{
			header('Content-Length: '.$st['size']);
			$lastm = gmstrftime('%a, %d %b %Y %H:%M:%S',
					$st['mtime']);
			header('Last-Modified: '.$lastm);
		}
		//XXX fpassthru() would be better but allocates too much memory
		while(!feof($fp))
			if(($buf = fread($fp, 65536)) !== FALSE)
				print($buf);
		fclose($fp);
	}

	private function renderString($string, $type)
	{
		header('Content-Length: '.strlen($string));
		print($string);
	}


	//protected
	//properties
	protected $request = FALSE;
}

?>
