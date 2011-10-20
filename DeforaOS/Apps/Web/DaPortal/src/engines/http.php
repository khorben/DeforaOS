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



require_once('./system/engine.php');
require_once('./system/page.php');
require_once('./system/template.php');


//HttpEngine
class HttpEngine extends Engine
{
	//public
	//methods
	//accessors
	//HttpEngine::getRequest
	public function getRequest()
	{
		if(!isset($_REQUEST))
			return FALSE;
		$module = FALSE;
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$parameters = FALSE;
		foreach($_REQUEST as $key => $value)
		{
			//FIXME disabled until all SQL queries are prepared
			//if(get_magic_quotes_gpc() != 0)
			//	$value = stripslashes($value);
			switch($key)
			{
				case 'module':
				case 'action':
				case 'id':
				case 'title':
					$$key = $_REQUEST[$key];
					break;
				default:
					if($parameters === FALSE)
						$parameters = array();
					$parameters[$key] = $value;
					break;
			}
		}
		return new Request($this, $module, $action, $id, $title,
				$parameters);
	}


	//useful
	//HttpEngine::render
	public function render($page)
	{
		if($page === FALSE)
			return FALSE;
		if(($template = Template::attachDefault($this)) === FALSE)
			return FALSE;
		header('Content-Type: '.$this->getType());
		$template->render($this, $page);
	}


	public function log($priority, $message)
	{
		parent::log($priority, $message);
		switch($priority)
		{
			case 'LOG_ERR':
				$dialog = new PageElement('dialog');
				$dialog->setProperty('title', 'Error');
				$dialog->setProperty('text', $message);
				if(strcmp($message, 'Permission denied') == 0)
				{
					$button = $dialog->append('button');
					$button->setProperty('text',
							'Authenticate');
				}
				return($dialog);
		}
		return FALSE;
	}


	//protected
	//methods
	protected function match()
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


	protected function attach()
	{
		$this->setType('text/html');
		$uri = isset($_SERVER['HTTPS']) ? 'https://' : 'http://';
		if(isset($_SERVER['HTTP_HOST']))
			$uri .= $_SERVER['HTTP_HOST'];
		else if(isset($_SERVER['SERVER_NAME']))
			$uri .= $_SERVER['SERVER_NAME'];
		else
			exit(-1);
		$port = isset($_SERVER['SERVER_PORT'])
			&& is_numeric($_SERVER['SERVER_PORT'])
			? $_SERVER['SERVER_PORT']
			: (isset($_SERVER['HTTPS']) ? 443 : 80);
		if(isset($_SERVER['HTTPS']) && $port == 443)
			$uri .= '/';
		else if(!isset($_SERVER['HTTPS']) && $port == 80)
			$uri .= '/';
		else
			$uri .= ':'.$port.'/';
		if(!isset($_SERVER['SCRIPT_NAME']))
			exit(-1);
		$name = ltrim($_SERVER['SCRIPT_NAME'], '/');
		$parent = $uri;
		if(dirname($name) != '.')
			$parent .= dirname($name);
		$uri .= $name;
		$this->log('LOG_DEBUG', 'URI is '.$uri);
		if(!isset($_SERVER['SCRIPT_NAME'])
				|| substr($_SERVER['SCRIPT_NAME'], -10)
				!= '/index.php')
		{
			header('Location: '.$parent);
			exit(0);
		}
	}


	//private
	//properties
	private $uri;
}

?>
