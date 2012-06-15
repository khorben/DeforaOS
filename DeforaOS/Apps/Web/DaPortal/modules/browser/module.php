<?php //$Id$
//Copyright (c) 2007-2008,2011-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));
$text = array();
$text['BROWSER'] = 'Browser';
$text['BROWSER_ADMINISTRATION'] = 'Browser administration';
$text['BYTES'] = 'bytes';
$text['GB'] = 'GB';
$text['GROUP'] = 'Group';
$text['INDEX_OF'] = 'Index of';
$text['KB'] = 'KB';
$text['MB'] = 'MB';
$text['OWNER'] = 'Owner';
$text['PARENT_DIRECTORY'] = 'Parent directory';
$text['SETTINGS'] = 'Settings';
$text['SIZE'] = 'Size';
$text['TB'] = 'TB';
global $lang;
if($lang == 'fr')
{
	$text['BROWSER'] = 'Explorateur';
	$text['BROWSER_ADMINISTRATION'] = "Administration de l'explorateur";
	$text['BYTES'] = 'octets';
	$text['GB'] = 'Go';
	$text['GROUP'] = 'Groupe';
	$text['INDEX_OF'] = 'Fichiers de';
	$text['KB'] = 'Ko';
	$text['MB'] = 'Mo';
	$text['OWNER'] = 'Propriétaire';
	$text['PARENT_DIRECTORY'] = 'Répertoire parent';
	$text['SETTINGS'] = 'Paramètres';
	$text['SIZE'] = 'Taille';
	$text['TB'] = 'To';
}
_lang($text);
define('S_IFDIR', 040000);


//BrowserModule
class BrowserModule extends Module
{
	//public
	//methods
	//useful
	//BrowserModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'download':
			case 'system':
				return $this->$action($args);
			case 'config_update':
				return $this->configUpdate($args);
			default:
				return $this->_default($args);
		}
	}


	//private
	private function _get_date($time)
	{
		return date('Y-m-d h:m:s', $time);
	}


	private function _get_group($id)
	{
		static $cache = array();

		if(isset($cache[$id]))
			return $cache[$id];
		if(($gr = posix_getgrgid($id)) == FALSE)
			$cache[$id] = $id;
		else
			$cache[$id] = $gr['name'];
		return $cache[$id];
	}


	private function _get_size($size)
	{
		if($size < 1024)
			return $size.' '.BYTES;
		if(($size = round($size / 1024)) < 1024)
			return $size.' '.KB;
		if(($size = round($size / 1024)) < 1024)
			return $size.' '.MB;
		if(($size = round($size / 1024)) < 1024)
			return $size.' '.GB;
		$size = round($size / 1024);
		return $size.' '.TB;
	}


	private function _get_user($id)
	{
		static $cache = array();

		if(isset($cache[$id]))
			return $cache[$id];
		if(($pw = posix_getpwuid($id)) == FALSE)
			$cache[$id] = $id;
		else
			$cache[$id] = $pw['name'];
		return $cache[$id];
	}


	//public
	//BrowserModule::admin
	protected function admin($args)
	{
		global $user_id;

		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title browser">'
				._html_safe(BROWSER_ADMINISTRATION)."</h1>\n");
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		if(($configs = _config_list('browser')))
		{
			$module = 'browser';
			$action = 'config_update';
			include('./system/config.tpl');
		}
	}


	//BrowserModule::config_update
	protected function configUpdate($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return $this->admin(array());
	}


	//BrowserModule::default
	protected function _default($args)
	{
		$file = $this->_defaultFile(isset($args['file'])
				? $args['file'] : '');
		if(($root = _config_get('browser', 'root')) == FALSE)
			return _error('Internal error');
		if(($st = stat($root.'/'.$file)) == FALSE)
			return _error('Could not open file');
		if(($st['mode'] & S_IFDIR) == S_IFDIR)
			return $this->_defaultDir($root, $file,
					isset($args['sort'])
					? $args['sort'] : FALSE);
		return $this->_default_display($root, $file);
	}

	private function _defaultFile($file)
	{
		$file = stripslashes($file);
		$filev = explode('/', $file);
		$file = '';
		foreach($filev as $f)
			if($f == '.' || $f == '..')
				continue;
			else
				$file .= '/'.$f;
		return '/'.trim($file, '/');
	}

	private function _defaultDir(&$root, &$file, $sort)
	{
		if(($dir = opendir($root.'/'.$file)) == FALSE)
			return _error('Could not open directory');
		print('<h1 class="title browser">'
				._html_safe(INDEX_OF.' '.$file)."</h1>\n");
		$entries = array();
		require_once('./system/mime.php');
		while(($de = readdir($dir)) != FALSE)
		{
			if(strcmp($de, '..') == 0 || $de[0] == '.')
				continue; /* FIXME latter should be optional */
			$entry = array('module' => 'browser',
					'action' => 'default',
					'id' => $file.'/'.$de, 'name' => $de,
					'icon' => 'icons/16x16/mime/default.png',
					'thumbnail' => 'icons/48x48/mime/default.png');
			$entry['id'] = '/'.trim($entry['id'], '/');
			if(($st = lstat($root.'/'.$entry['id'])) == FALSE)
			{
				$entries[] = $entry;
				continue;
			}
			$entry['args'] = 'file='.$entry['id'];
			$entry['uid'] = $this->_get_user($st['uid']);
			$entry['gid'] = $this->_get_group($st['gid']);
			$entry['size'] = $st['size'];
			$entry['dsize'] = $this->_get_size($st['size']);
			$entry['mtime'] = $this->_get_date($st['mtime']);
			if(($st['mode'] & 040000) == 040000)
			{
				$entry['icon'] = 'icons/16x16/mime/folder.png';
				$entry['thumbnail'] = 'icons/48x48/mime/folder.png';
			}
			else
			{
				$mime = _mime_from_ext($de);
				$entry['icon'] = 'icons/16x16/mime/'.$mime.'.png';
				if(!is_readable($entry['icon']))
					$entry['icon'] = 'icons/16x16/mime/default.png';
				$entry['thumbnail'] = 'icons/48x48/mime/'.$mime.'.png';
				if(!is_readable($entry['thumbnail']))
					$entry['thumbnail'] = 'icons/48x48/mime/default.png';
			}
			$entries[] = $entry;
		}
		$fsort = $sort;
		switch($sort)
		{
			case 'dsize':	$fsort = 'size';	break;
			case 'gid':				break;
			case 'mtime':				break;
			case 'uid':				break;
			case 'name':				break;
			default:	$fsort = 'name';
					$sort = 'name';		break;
		}
		$func = '_entries_sort_'.$fsort;
		usort($entries, array($this, $func));
		$toolbar = array();
		$toolbar[] = array('title' => BACK, 'class' => 'back',
				'onclick' => 'history.back(); return false');
		$toolbar[] = array('class' => 'parent_directory',
				'link' => _html_link('browser', FALSE, FALSE,
					FALSE, 'file='.dirname($file)),
				'title' => PARENT_DIRECTORY);
		$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
				'onclick' => 'history.forward(); return false');
		$class = array('dsize' => SIZE, 'uid' => OWNER, 'gid' => GROUP,
				'mtime' => DATE);
		_module('explorer', 'browse', array('toolbar' => $toolbar,
					'view' => 'details',
					'entries' => $entries,
					'class' => $class,
					'module' => 'browser',
					'action' => 'default',
					'sort' => $sort));
	}

	private function _default_display(&$root, &$file)
	{
		include('./modules/browser/display.tpl');
	}

	private function _entries_sort_gid(&$a, &$b)
	{
		return strcmp($a['gid'], $b['gid']);
	}

	private function _entries_sort_mtime(&$a, &$b)
	{
		return strcmp($a['mtime'], $b['mtime']);
	}

	private function _entries_sort_name(&$a, &$b)
	{
		return strcmp($a['id'], $b['id']);
	}

	private function _entries_sort_size(&$a, &$b)
	{
		return $a['size'] - $b['size'];
	}

	private function _entries_sort_uid(&$a, &$b)
	{
		return strcmp($a['uid'], $b['uid']);
	}


	//BrowserModule::download
	protected function download($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return $this->_default($args);
	}


	//BrowserModule::system
	protected function system($args)
	{
		global $title, $error;

		$title.=' - '.BROWSER;
		if($_SERVER['REQUEST_METHOD'] == 'GET')
		{
			if(!isset($args['action']))
				$args['action'] = 'default';
			switch($args['action'])
			{
				case 'admin':
					break;
				case 'download':
					if(!isset($args['file']))
						break;
					$error = $this->_systemDownload(
							$args['file']);
					break;
			}
		}
		else if($_SERVER['REQUEST_METHOD'] == 'POST')
		{
			switch($args['action'])
			{
				case 'config_update':
					$error = $this->_systemConfigUpdate(
							$args);
					break;
			}
		}
	}

	private function _systemConfigUpdate($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		_config_update('browser', $args);
		header('Location: '._module_link('browser', 'admin'));
		exit(0);
	}

	private function _systemDownload($file)
	{
		$file = $this->_defaultFile($file);
		if(($root = _config_get('browser', 'root')) == FALSE)
			return 'Internal error';
		if(($fp = fopen($root.'/'.$file, 'rb')) == FALSE)
			return 'Could not open file';
		if(($st = fstat($fp)) == FALSE || ($st['mode'] & 040000)
				== 040000)
		{
			fclose($fp);
			return 'Is a directory';
		}
		header('Content-Length: '.$st['size']);
		require_once('./system/mime.php');
		$mime = _mime_from_ext(addslashes($file));
		header('Content-Type: '.$mime);
		$file = basename($file);
		if(strpos($file, "\n") === FALSE)
			//FIXME escape filename
			header('Content-Disposition: attachment; filename="'
					.$file.'"');
		fpassthru($fp);
		fclose($fp);
		exit(0);
	}
}

?>
