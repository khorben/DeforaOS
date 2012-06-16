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



require_once('./system/module.php');


//BrowserModule
class BrowserModule extends Module
{
	//public
	//methods
	//essential
	//BrowserModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'default':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
			default:
				return FALSE;
		}
	}


	//protected
	//methods
	//accessors
	//BrowserModule::getRoot
	protected function getRoot($engine)
	{
		global $config;
		$error = 'The browser repository is not configured';

		if(($root = $config->getVariable('module::'.$this->name,
				'root')) === FALSE)
		{
			$engine->log('LOG_WARNING', $error);
			$root = '/tmp';
		}
		return $root;
	}


	//useful
	//calls
	protected function callDefault($engine, $request)
	{
		$root = $this->getRoot($engine);

		//verify request
		if(($folder = $request->getTitle()) === FALSE)
			$folder = '/';
		else
			$folder = '/'.ltrim($folder, '/');
		$folder = $this->helperSanitizePath($folder);
		$title = _('Browser: ').$folder;
		$page = new Page(array('title' => $title));
		//title
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//view
		$view = $page->append('treeview');
		if(($dir = opendir($root.'/'.$folder)) !== FALSE)
			while(($de = readdir($dir)) !== FALSE)
			{
				if($de == '.' || $de == '..')
					continue;
				$row = $view->append('row');
				$row->setProperty('title', $de);
			}
		return $page;
	}


	//helpers
	protected function helperSanitizePath($path)
	{
		$path = str_replace('/./', '/', $path);
		//FIXME really implement '..'
		if(strpos($path, '/../') !== FALSE)
			return '/';
		return $path;
	}
}

?>
