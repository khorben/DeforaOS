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
			case 'download':
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
	//BrowserModule::callDefault
	protected function callDefault($engine, $request)
	{
		//obtain the path requested
		if(($path = $request->getTitle()) === FALSE)
			$path = '/';
		$path = $this->helperSanitizePath($path);
		$title = _('Browser: ').$path;
		$page = new Page(array('title' => $title));
		//title
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//view
		$this->helperDisplay($engine, $page, $path);
		return $page;
	}


	//BrowserModule::callDownload
	protected function callDownload($engine, $request)
	{
		$root = $this->getRoot($engine);
		$error = _('Could not download the file requested');

		//obtain the path requested
		if(($path = $request->getTitle()) === FALSE)
			$path = '/';
		$path = $this->helperSanitizePath($path);
		$title = _('Browser: ').$path;
		if(($fp = fopen($root.'/'.$path, 'rb')) !== FALSE)
			return $fp;
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$page->append('dialog', array('type' => 'error',
				'text' => $error));
		return $page;
	}


	//helpers
	//BrowserModule::helperDisplay
	protected function helperDisplay($engine, $page, $path)
	{
		$root = $this->getRoot($engine);
		$error = _('Could not open the file or directory requested');

		if(($st = lstat($root.'/'.$path)) === FALSE)
			return $page->append('dialog', array('type' => 'error',
					'text' => $error));
		if($st['mode'] & 040000 == 040000)
			$this->helperDisplayDirectory($engine, $page, $root,
					$path);
		else
			$this->helperDisplayFile($engine, $page, $root, $path);
	}


	//BrowserModule::helperDisplayDirectory
	protected function helperDisplayDirectory($engine, $page, $root, $path)
	{
		$error = _('Could not open the directory requested');

		if(($dir = @opendir($root.'/'.$path)) === FALSE)
			return $page->append('dialog', array('type' => 'error',
				'text' => $error));
		$view = $page->append('treeview');
		while(($de = readdir($dir)) !== FALSE)
		{
			//skip "." and ".."
			if($de == '.' || $de == '..')
				continue;
			$fullpath = $root.'/'.$path.'/'.$de;
			$st = lstat($fullpath);
			$row = $view->append('row');
			$r = new Request($engine, $this->name, FALSE,
					FALSE, ltrim($path.'/'.$de, '/'));
			$link = new PageElement('link', array(
					'request' => $r, 'text' => $de));
			$row->setProperty('title', $link);
		}
	}


	//BrowserModule::helperDisplayFile
	protected function helperDisplayFile($engine, $page, $root, $path, $st)
	{
		$hbox = $page->append('hbox');
		$col1 = $hbox->append('vbox');
		$col2 = $hbox->append('vbox');
		//filename
		$col1->append('label', array('class' => 'bold',
				'text' => _('Filename: ')));
		$r = new Request($engine, $this->name, 'download', FALSE,
				$path);
		$link = new PageElement('link', array('request' => $r,
				'text' => basename($path)));
		$col2->append($link);
	}


	//BrowserModule::helperSanitizePath
	protected function helperSanitizePath($path)
	{
		$path = '/'.ltrim($path, '/');
		$path = str_replace('/./', '/', $path);
		//FIXME really implement '..'
		if(strpos($path, '/../') !== FALSE)
			return '/';
		return $path;
	}
}

?>
