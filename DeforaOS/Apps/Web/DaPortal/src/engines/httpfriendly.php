<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
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



require_once('./engines/http.php');


//HttpFriendlyEngine
class HttpFriendlyEngine extends HttpEngine
{
	//public
	//methods
	//essential
	//HttpFriendlyEngine::match
	public function match()
	{
		if(($score = parent::match()) != 100)
			return $score;
		return $score + 1;
	}


	//accessors
	//HttpFriendlyEngine::getRequest
	public function getRequest()
	{
		if(!isset($_SERVER['PATH_INFO']))
			return parent::getRequest();
		$path = explode('/', $_SERVER['PATH_INFO']);
		if(!is_array($path) || count($path) < 2)
			return parent::getRequest();
		array_shift($path);
		$module = array_shift($path);
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		$args = FALSE;
		if(count($path) > 0)
		{
			$id = array_shift($path);
			if(!is_numeric($id))
			{
				$action = $id;
				$id = FALSE;
				if(count($path) > 0)
				{
					$id = array_shift($path);
					if(!is_numeric($id))
						return parent::getRequest();
				}
			}
		}
		if(count($path) > 0)
			$title = array_shift($path);
		if(count($path) != 0)
			return parent::getRequest();
		//FIXME implement arguments
		return new Request($this, $module, $action, $id, $title);
	}


	//HttpFriendlyEngine::getUrl
	public function getUrl($request, $absolute = TRUE)
	{
		global $config;

		//FIXME do not include parameters for a POST request
		if($request === FALSE)
			return FALSE;
		$name = $_SERVER['SCRIPT_NAME'];
		//use the kicker instead if defined
		if(($kicker = $config->getVariable('engine::httpfriendly',
				'kicker')) !== FALSE)
			$name = dirname($name).'/'.$kicker;
		$name = ltrim($name, '/');
		if($absolute)
		{
			//prepare the complete address
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
			//prepare a relative address
			$url = basename($name);
		//return if already complete
		if(($module = $request->getModule()) === FALSE)
			return $url;
		//handle the main parameters
		$url .= '/'.urlencode($module);
		if(($action = $request->getAction()) !== FALSE)
			$url .= '/'.urlencode($action);
		if(($id = $request->getId()) !== FALSE)
			$url .= '/'.urlencode($id);
		if(($title = $request->getTitle()) !== FALSE)
		{
			$title = str_replace(' ', '-', $title);
			$url .= '/'.urlencode($title);
		}
		//handle arguments
		if($request->isIdempotent()
				&& ($args = $request->getParameters())
				!== FALSE)
		{
			$sep = '?';
			foreach($args as $key => $value)
			{
				$url .= $sep.urlencode($key)
					.'='.urlencode($value);
				$sep = '&';
			}
		}
		return $url;
	}
}

?>
