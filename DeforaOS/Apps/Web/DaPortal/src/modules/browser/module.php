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



require_once('./system/common.php');
require_once('./system/mime.php');
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
			case 'actions':
				return $this->$action($engine, $request);
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
	//BrowserModule::getDate
	protected function getDate($time)
	{
		return strftime(_('%Y/%m/%d %H:%M:%S'), $time);
	}


	//BrowserModule::getGroup
	protected function getGroup($gid)
	{
		static $cache = array();

		if(isset($cache[$gid]))
			return $cache[$gid];
		$cache[$gid] = (($gr = posix_getgrgid($gid)) !== FALSE)
			? $gr['name'] : $gid;
		return $cache[$gid];
	}


	//BrowserModule::getPath
	protected function getPath($engine, $request)
	{
		$root = $this->getRoot($engine);

		if(($path = $request->getTitle()) === FALSE)
			return '/';
		$p = str_replace('-', '?', $path);
		if(($res = glob($root.'/'.$p)) !== FALSE && count($res) == 1)
			$path = substr($res[0], strlen($root));
		return $this->helperSanitizePath($path);
	}


	//BrowserModule::getPermissions
	protected function getPermissions($mode)
	{
		return Common::getPermissions($mode);
	}


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


	//BrowserModule::getToolbar
	protected function getToolbar($engine, $path, $directory = FALSE)
	{
		$toolbar = new PageElement('toolbar');

		if(($parent = dirname($path)) != $path)
		{
			$r = new Request($engine, $this->name, FALSE, FALSE,
					ltrim($parent, '/'));
			//XXX change the label to "Browse" for files
			$toolbar->append('button', array('request' => $r,
					'stock' => 'updir',
					'text' => _('Parent directory')));
		}
		$r = new Request($engine, $this->name, FALSE, FALSE,
				ltrim($path, '/'));
		$toolbar->append('button', array('request' => $r,
				'stock' => 'refresh', 'text' => _('Refresh')));
		if($directory === FALSE)
		{
			$r = new Request($engine, $this->name, 'download',
					FALSE, ltrim($path, '/'));
			$toolbar->append('button', array('request' => $r,
					'stock' => 'download',
					'text' => _('Download')));
		}
		return $toolbar;
	}


	//BrowserModule::getUser
	protected function getUser($uid)
	{
		static $cache = array();

		if(isset($cache[$uid]))
			return $cache[$uid];
		$cache[$uid] = (($pw = posix_getpwuid($uid)) !== FALSE)
			? $pw['name'] : $uid;
		return $cache[$uid];
	}


	//useful
	//actions
	//BrowserModule::actions
	protected function actions($engine, $request)
	{
		$ret = array();
		if($request->getParameter('user') !== FALSE)
			return $ret;
		if($request->getParameter('admin') != 0)
			return $ret;
		return $ret;
	}


	//calls
	//BrowserModule::callDefault
	protected function callDefault($engine, $request)
	{
		//obtain the path requested
		$path = $this->getPath($engine, $request);
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
		$path = $this->getPath($engine, $request);
		if(($fp = fopen($root.'/'.$path, 'rb')) !== FALSE)
		{
			$mime = Mime::getType($engine, $path);
			$engine->setType($mime);
			return $fp;
		}
		$title = _('Browser: ').$path;
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

		//FIXME let stat() vs lstat() be configurable
		if(($st = @stat($root.'/'.$path)) === FALSE)
			return $page->append('dialog', array('type' => 'error',
					'text' => $error));
		if(($st['mode'] & Common::$S_IFDIR) == Common::$S_IFDIR)
		{
			$toolbar = $this->getToolbar($engine, $path, TRUE);
			$page->append($toolbar);
			$this->helperDisplayDirectory($engine, $page, $root,
					$path);
		}
		else
		{
			$toolbar = $this->getToolbar($engine, $path, FALSE);
			$page->append($toolbar);
			$this->helperDisplayFile($engine, $page, $root, $path,
					$st);
		}
	}


	//BrowserModule::helperDisplayDirectory
	protected function helperDisplayDirectory($engine, $page, $root, $path)
	{
		$error = _('Could not open the directory requested');

		if(($dir = @opendir($root.'/'.$path)) === FALSE)
			return $page->append('dialog', array('type' => 'error',
				'text' => $error));
		$path = rtrim($path, '/');
		$columns = array('icon' => '', 'title' => _('Title'),
				'user' => _('User'), 'group' => _('Group'),
				'size' => _('Size'), 'date' => _('Date'),
				'mode' => _('Permissions'));
		$view = $page->append('treeview', array('columns' => $columns));
		while(($de = readdir($dir)) !== FALSE)
		{
			//skip "." and ".."
			if($de == '.' || $de == '..')
				continue;
			$fullpath = $root.'/'.$path.'/'.$de;
			$st = lstat($fullpath);
			$row = $view->append('row');
			if($st['mode'] & Common::$S_IFDIR)
				$icon = Mime::getIconByType($engine,
							'inode/directory', 16);
			else
				$icon = Mime::getIcon($engine, $de, 16);
			$icon = new PageElement('image', array(
					'source' => $icon));
			$row->setProperty('icon', $icon);
			$r = new Request($engine, $this->name, FALSE,
					FALSE, ltrim($path.'/'.$de, '/'));
			$link = new PageElement('link', array(
					'request' => $r, 'text' => $de));
			$row->setProperty('title', $link);
			$row->setProperty('user', $this->getUser($st['uid']));
			$row->setProperty('group', $this->getGroup($st['gid']));
			$row->setProperty('size', Common::getSize($st['size']));
			$row->setProperty('date', $this->getDate($st['mtime']));
			//permissions
			$permissions = $this->getPermissions($st['mode']);
			$permissions = new PageElement('label', array(
					'class' => 'preformatted',
					'text' => $permissions));
			$row->setProperty('mode', $permissions);
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
				'text' => _('Filename:')));
		$r = new Request($engine, $this->name, 'download', FALSE,
				ltrim($path, '/'));
		$link = new PageElement('link', array('request' => $r,
				'text' => basename($path)));
		$col2->append($link);
		//type
		$this->_displayFileField($col1, $col2, _('Type:'),
				Mime::getType($engine, basename($path)));
		//user
		$this->_displayFileField($col1, $col2, _('User:'),
				$this->getUser($st['uid']));
		//group
		$this->_displayFileField($col1, $col2, _('Group:'),
				$this->getGroup($st['gid']));
		//permissions
		$this->_displayFileField($col1, $col2, _('Permissions:'),
				$this->getPermissions($st['mode']));
		//size
		$this->_displayFileField($col1, $col2, _('Size:'),
				Common::getSize($st['size']));
		//creation time
		$this->_displayFileField($col1, $col2, _('Created on:'),
				$this->getDate($st['ctime']));
		//modification time
		$this->_displayFileField($col1, $col2, _('Last modified:'),
				$this->getDate($st['mtime']));
		//access time
		$this->_displayFileField($col1, $col2, _('Last access:'),
				$this->getDate($st['atime']));
	}

	private function _displayFileField($col1, $col2, $field, $value)
	{
		$col1->append('label', array('class' => 'bold',
				'text' => $field.' '));
		$col2->append('label', array('text' => $value));
	}


	//BrowserModule::helperSanitizePath
	protected function helperSanitizePath($path)
	{
		$path = '/'.ltrim($path, '/');
		$path = str_replace('/./', '/', $path);
		//FIXME really implement '..'
		if(strcmp($path, '/..') == 0 || strpos($path, '/../') !== FALSE)
			return '/';
		return $path;
	}
}

?>
