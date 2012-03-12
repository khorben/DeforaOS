<?php //$Id$
//Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org>
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
//FIXME
//- permissions are not implemented



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['ACCESS_TIME'] = 'Access time';
$text['BACK'] = 'Back';
$text['BROWSE'] = 'Browse';
$text['COMMENT'] = 'Comment';
$text['CREATE'] = 'Create';
$text['CREATION_TIME'] = 'Creation time';
$text['DOWNLOAD'] = 'Download';
$text['DOWNLOADS_ADMINISTRATION'] = 'Downloads administration';
$text['DOWNLOADS_LIST'] = 'Downloads list';
$text['FILE'] = 'File';
$text['FORWARD'] = 'Forward';
$text['IMAGE_PREVIEW'] = 'Image preview';
$text['MODE'] = 'Permissions';
$text['MODIFICATION_TIME'] = 'Modification time';
$text['NAME'] = 'Name';
$text['NEW_DIRECTORY'] = 'New directory';
$text['OWNER'] = 'Owner';
$text['PARENT_DIRECTORY'] = 'Parent directory';
$text['PERMISSIONS'] = 'Permissions';
$text['SETTINGS'] = 'Settings';
$text['SIZE'] = 'Size';
$text['TYPE'] = 'Type';
$text['UPLOAD'] = 'Upload';
$text['UPLOAD_FILE'] = 'Upload file';
$text['UPDATE'] = 'Update';
$text['UPDATE_FILE'] = 'Update file';
global $lang;
if($lang == 'fr')
{
	include('./modules/download/lang.fr.php');
}
_lang($text);
define('S_IFDIR', 01000);


//DownloadModule
class DownloadModule extends Module
{
	//public
	//methods
	//useful
	//DownloadModule::call
	public function call(&$engine, $request)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'download':
			case 'file_insert':
			case 'system':
				return $this->$action($args);
			default:
				return $this->_default($args);
		}
		return FALSE;
	}


	//private
	//DownloadModule::_permissions
	private function _permissions($mode)
	{
		$str = '----------';
		if(($mode & S_IFDIR) == S_IFDIR)
			$str[0] = 'd';
		$str[1] = $mode & 0400 ? 'r' : '-';
		$str[2] = $mode & 0200 ? 'w' : '-';
		$str[3] = $mode & 0100 ? 'x' : '-';
		$str[4] = $mode & 040 ? 'r' : '-';
		$str[5] = $mode & 020 ? 'w' : '-';
		$str[6] = $mode & 010 ? 'x' : '-';
		$str[7] = $mode & 04 ? 'r' : '-';
		$str[8] = $mode & 02 ? 'w' : '-';
		$str[9] = $mode & 01 ? 'x' : '-';
		return $str;
	}


	//DownloadModule::_file_get
	private function _file_get($id)
	{
		if(!is_numeric($id))
			return FALSE;
		if(!($root = _config_get('download', 'root')))
			return FALSE;
		$query = 'SELECT title AS name, daportal_content.content_id'
			.' AS id, download_id, parent, content AS comment'
			.' FROM daportal_download, daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			." AND enabled='1'"
			." AND daportal_content.content_id='$id'";
		$file = _sql_array($query);
		if(!is_array($file) || count($file) != 1
				|| !is_numeric($file[0]['download_id']))
			return FALSE;
		$file = $file[0];
		$file['filename'] = $root.'/'.$file['download_id'];
		if(!is_readable($file['filename']))
			return FALSE;
		return $file;
	}


	//public
	//DownloadModule::admin
	public function admin($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title download">'._html_safe(
					DOWNLOADS_ADMINISTRATION)."</h1>\n"
				.'<h2 class="title settings">'
				._html_safe(SETTINGS)."</h2>\n");
		if(($configs = _config_list('download')))
		{
			$module = 'download';
			$action = 'config_update';
			include('./system/config.tpl');
		}
		print('<h2 class="title download">'._html_safe(DOWNLOADS_LIST)
				."</h2>\n");
		$query = 'SELECT daportal_content.content_id AS id'
			.', title AS name, daportal_content.enabled AS enabled'
			.', username AS owner, mode'
			.' FROM daportal_download, daportal_content'
			.', daportal_user WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' ORDER BY id DESC';
		$dls = _sql_array($query);
		if(!is_array($dls))
			return _error('Unable to list downloads');
		for($cnt = count($dls), $i = 0; $i < $cnt; $i++)
		{
			$dls[$i]['icon'] = ($dls[$i]['mode'] & S_IFDIR)
				? 'folder.png' : 'default.png';
			$dls[$i]['thumbnail'] = 'icons/48x48/mime/'
				.$dls[$i]['icon'];
			$dls[$i]['icon'] = 'icons/16x16/mime/'.$dls[$i]['icon'];
			$dls[$i]['module'] = 'download';
			$dls[$i]['action'] = 'default';
			$dls[$i]['apply_module'] = 'download';
			$dls[$i]['apply_id'] = $dls[$i]['id'];
			$dls[$i]['name'] = _html_safe($dls[$i]['name']);
			$dls[$i]['enabled'] = ($dls[$i]['enabled'] == SQL_TRUE)
				? 'enabled' : 'disabled';
			$dls[$i]['enabled'] = '<img src="icons/16x16/'
				.$dls[$i]['enabled'].'.png" alt="'
				.$dls[$i]['enabled'].'.png" title="'
				.(($dls[$i]['enabled'] == 'enabled')
						? ENABLED : DISABLED).'"/>';
			$dls[$i]['owner'] = _html_safe($dls[$i]['owner']);
			$dls[$i]['mode'] = _html_safe(
					$this->_permissions($dls[$i]['mode']));
		}
		$toolbar = array();
		$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
				'action' => 'enable');
		$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
				'action' => 'disable');
		$toolbar[] = array('title' => DELETE, 'class' => 'delete',
				'action' => 'delete', 'confirm' => 'delete');
		$toolbar[] = array();
		$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
				'link' => _module_link('download', 'admin'),
				'onclick' => 'location.reload(); return false');
		_module('explorer', 'browse_trusted', array('entries' => $dls,
					'class' => array('enabled' => ENABLED,
						'owner' => OWNER,
						'mode' => MODE),
					'toolbar' => $toolbar,
					'view' => 'details',
					'module' => 'download',
					'action' => 'admin'));
	}


	function download_config_update($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return download_admin(array());
	}


	//DownloadModule::_default
	public function _default($args)
	{
		global $user_id;

		$parent = ' IS NULL';
		$sql = 'SELECT daportal_content.content_id AS id, download_id'
			.', title AS name, mode, parent, timestamp AS ctime'
			.', content, daportal_content.user_id AS user_id'
			.', username AS user'
			.' FROM daportal_download, daportal_content'
			.', daportal_user'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_user.enabled='1' AND ";
		if(isset($args['id']) && is_numeric($args['id'])
				|| (isset($args['download_id'])
					&& is_numeric($args['download_id'])))
		{
			$file = (isset($args['download_id'])
					&& is_numeric($args['download_id']))
				? _sql_array($sql." download_id='".$args['download_id']
						."'")
				: _sql_array($sql." daportal_content.content_id='"
						.$args['id']."'");
			if(is_array($file) && count($file) == 1)
			{
				$file = $file[0];
				if(!($file['mode'] & S_IFDIR))
					return $this->_default_download($file);
				$parent = "='".$file['download_id']."'";
			}
		}
		if(!isset($file) || !is_array($file) || count($file) == 0)
			$file = array('id' => '');
		print('<h1 class="title download">'._html_safe(DOWNLOADS));
		if(isset($file['name']))
			print(': '._html_safe($file['name']));
		print("</h1>\n");
		$sql = 'SELECT daportal_content.content_id AS id, title AS name'
			.', daportal_content.enabled AS enabled'
			.', timestamp AS date, mode'
			.', daportal_content.user_id AS user_id, username'
			.' FROM daportal_download, daportal_content'
			.', daportal_user'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND parent$parent ORDER BY name";
		$dls = _sql_array($sql);
		if(!is_array($dls))
			return _error('Unable to list downloads');
		require_once('./system/mime.php');
		for($cnt = count($dls), $i = 0; $i < $cnt; $i++)
		{
			$dls[$i]['module'] = 'download';
			$dls[$i]['action'] = 'default';
			$dls[$i]['apply_module'] = 'download';
			$dls[$i]['apply_id'] = $dls[$i]['id'];
			$dls[$i]['username'] = '<a href="'._html_link('user', FALSE,
				$dls[$i]['user_id'], $dls[$i]['username']).'">'
					.$dls[$i]['username'].'</a>';
			$mime = _mime_from_ext($dls[$i]['name']);
			if($dls[$i]['mode'] & S_IFDIR)
			{
				$dls[$i]['icon'] = 'icons/16x16/mime/folder.png';
				$dls[$i]['thumbnail'] = 'icons/48x48/mime/folder.png';
				$dls[$i]['mode'] = _html_safe($this->_permissions(
							$dls[$i]['mode']));
				continue;
			}
			$dls[$i]['thumbnail'] = is_readable('icons/48x48/mime/'.$mime
					.'.png') ? 'icons/48x48/mime/'.$mime.'.png'
				: 'icons/48x48/mime/default.png';
			$dls[$i]['icon'] = is_readable('icons/16x16/mime/'.$mime.'.png')
				? 'icons/16x16/mime/'.$mime.'.png'
				: $dls[$i]['thumbnail'];
			/* XXX format the date */
			$dls[$i]['date'] = _html_safe($dls[$i]['date']);
			$dls[$i]['mode'] = _html_safe($this->_permissions($dls[$i]['mode']));
		}
		$toolbar = array();
		$toolbar[] = array('title' => BACK, 'class' => 'back',
				'onclick' => 'history.back(); return false');
		$toolbar[] = array('title' => PARENT_DIRECTORY,
				'class' => 'parent_directory',
				'link' => _module_link('download', FALSE, FALSE,
					FALSE, isset($file['parent'])
					? 'download_id='.$file['parent'] : ''));
		$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
				'onclick' => 'history.forward(); return false');
		$toolbar[] = array();
		$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
				'link' => _module_link('download', FALSE,
				$file['id']),
				'onclick' => 'location.reload(); return false');
		require_once('./system/user.php');
		if(_user_admin($user_id))
		{
			$toolbar[] = array();
			$toolbar[] = array('title' => NEW_DIRECTORY,
					'class' => 'folder-new',
					'link' => _module_link('download',
						'directory_new', $file['id']));
			$toolbar[] = array('title' => UPLOAD_FILE,
					'class' => 'upload_file',
					'link' => _module_link('download',
						'file_new', $file['id']));
			$toolbar[] = array('title' => DELETE,
					'class' => 'delete',
					'action' => 'delete',
					'confirm' => 'delete');
		}
		_module('explorer', 'browse_trusted', array('entries' => $dls,
					'class' => array('username' => USER,
						'date' => DATE,
						'mode' => PERMISSIONS),
					'toolbar' => $toolbar,
					'module' => 'download',
					'action' => 'default',
					'id' => $file['id']));
	}

	private function _default_download($file)
	{
		//FIXME replace default icon
		if(!($root = _config_get('download', 'root')))
			return _error('No root directory');
		require_once('./system/mime.php');
		if(($file['mime'] = _mime_from_ext($file['name'])) == 'default')
			$file['mime'] = 'text/plain';
		$filename = $root.'/'.$file['download_id'];
		$st = stat($filename);
		$file['mode'] = $this->_permissions($file['mode']);
		$file['ctime'] = strftime(DATE_FORMAT, $st['ctime']);
		$file['mtime'] = strftime(DATE_FORMAT, $st['mtime']);
		$file['atime'] = strftime(DATE_FORMAT, $st['atime']);
		$file['size'] = round($st['size'] / 1024);
		$file['size'] = ($file['size'] > 1024)
			? round($file['size'] / 1024).' MB'
			: $file['size'].' KB';
		include('./modules/download/file_display.tpl');
	}


	function download_delete($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(!($root = _config_get('download', 'root')))
			return _error('No root directory');
		if(!is_numeric($args['id']))
			return _error(INVALID_ARGUMENT);
		$file = _sql_array('SELECT content_id AS id, download_id, mode'
				.' FROM daportal_download'
				." WHERE content_id='".$args['id']."'");
		if(!is_array($file) || count($file) != 1)
			return _error(INVALID_ARGUMENT);
		$file = $file[0];
		//FIXME if directory look for childs
		require_once('./system/content.php');
		$query = 'DELETE FROM daportal_download'
			." WHERE download_id='".$file['download_id']."'";
		_sql_query($query);
		_content_delete($file['id']);
		if(!($file['mode'] & S_IFDIR))
			unlink($root.'/'.$file['download_id']);
	}


	function download_disable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		$query = 'SELECT content_id FROM daportal_download'
			." WHERE content_id='".$args['id']."'";
		if(!($id = _sql_single($query)))
			return _error(INVALID_ARGUMENT);
		require_once('./system/content.php');
		_content_disable($id);
	}


	function download_directory_insert($args)
	{
		global $user_id;

		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(strlen($args['title']) == 0)
			return _error(INVALID_ARGUMENT);
		$parent = FALSE;
		if(isset($args['parent']))
			$parent = _sql_single('SELECT download_id'
					.' FROM daportal_download'
					." WHERE content_id='".$args['parent']."'");
		$parent = $parent != FALSE ? "'$parent'" : 'NULL';
		//FIXME not twice the same filename in a directory
		require_once('./system/content.php');
		if(!($id = _content_insert($args['title'], '', 1)))
			return _error('Unable to create directory');
		if(!_sql_query('INSERT INTO daportal_download'
					.' (content_id, parent, mode) VALUES'
					." ('$id', $parent, '".S_IFDIR."')"))
		{
			_content_delete($id);
			return _error('Unable to create directory');
		}
		download_default(array('id' => $id));
	}


	function download_directory_new($args)
	{
		global $user_id;

		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title directory">'._html_safe(NEW_DIRECTORY).'</h1>'
				."\n");
		$parent = isset($args['id']) ? $args['id'] : FALSE;
		include('./modules/download/directory_update.tpl');
	}


	//DownloadModule::download
	public function download($args)
	{
		if(!isset($args['id']))
			return _error(INVALID_ARGUMENT);
		if(($file = $this->_file_get($args['id'])) == FALSE)
			return _error(INVALID_ARGUMENT);
		if(($fp = fopen($file['filename'], 'r')) == FALSE)
			return _error('Unable to open file'); //FIXME 501
		require_once('./system/mime.php');
		if(($mime = _mime_from_ext($file['name'])) == 'default')
			$mime = 'text/plain';
		$client_mime = isset($_SERVER['HTTP_ACCEPT'])
			? explode(',', $_SERVER['HTTP_ACCEPT']) : array();
		for($i = 0; $i < count($client_mime); $i++) //FIXME should glob
		{
			if(($pos = strpos($client_mime[$i], ';')) == FALSE)
				continue;
			$client_mime[$i] = substr($client_mime[$i], 0, $pos);
		}
		$attachment = (strncmp('image/', $mime, 6) == 0
				|| in_array($mime, $client_mime))
			? 'inline' : 'attachment';
		header('Content-Type: '.$mime);
		header('Content-Length: '.filesize($file['filename']));
		header('Content-Disposition: '.$attachment.'; filename="'
				.addslashes($file['name']).'"');
		if(isset($_SERVER['HTTP_RANGE'])
				&& preg_match_all('/^bytes=([0-9]+)-$/',
					$_SERVER['HTTP_RANGE'], $offset))
		{
			$offset = $offset[1][0];
			if(fseek($fp, $offset) == 0)
			{
				$range = isset($st)
					? ($st['size'] - 1).'/'.$st['size']
					: '*/*';
					      header('Content-Range: bytes '
					      .$offset.'-'.$range);
					      }
					      }
		while(($buf = @fread($fp, 8192)) != FALSE)
			print($buf);
		fclose($fp);
	}


	function download_enable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(!($id = _sql_single('SELECT content_id FROM daportal_download'
				." WHERE content_id='".$args['id']."'")))
			return _error(INVALID_ARGUMENT);
		require_once('./system/content.php');
		_content_enable($id);
	}


	//DownloadModule::file_insert
	function file_insert($args)
	{
		global $user_id;

		//FIXME call from download_system() on POST and html if error
		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(!isset($_FILES['file']) || !isset($args['parent']))
			return _error(INVALID_ARGUMENT);
		$title = $_FILES['file']['name'];
		$comment = isset($args['comment']) ? $args['comment'] : '';
		if(!($root = _config_get('download', 'root')))
			return _error('No root directory');
		$parent = is_numeric($args['parent']) ? "'".$args['parent']."'"
			: 'NULL';
		//FIXME not twice the same filename in a directory
		require_once('./system/content.php');
		if(!($content_id = _content_insert($title, $comment, 1)))
			return _error('Unable to upload file');
		if(!_sql_query('INSERT INTO daportal_download (content_id, parent)'
			." VALUES ('$content_id', $parent)"))
		{
			_content_delete($content_id);
			return _error('Unable to upload file');
		}
		$id = _sql_id('daportal_download', 'download_id');
		if(!rename($_FILES['file']['tmp_name'], $root.'/'.$id))
		{
			_sql_query('DELETE FROM daportal_download'
			." WHERE download_id='$id'");
			_content_delete($content_id);
			return _error('Unable to rename file');
		}
		header('Location: '._module_link('download', FALSE, FALSE, FALSE,
			'download_id='.(is_numeric($args['parent'])
			? $args['parent'] : '')));
	}


	function download_file_new($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title download">'._html_safe(UPLOAD_FILE)."</h1>\n");
		if(isset($args['id']))
			$parent = _sql_single('SELECT download_id'
					.' FROM daportal_download WHERE content_id='
					."'".$args['id']."'");
		include('./modules/download/file_update.tpl');
	}


	function download_file_update($args)
	{
		global $user_id, $error;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		print('<h1 class="title download">'._html_safe(UPDATE_FILE)
				."</h1>\n");
		if(isset($error) && strlen($error))
			_error($error);
		if(!isset($args['id']))
			return _error(INVALID_ARGUMENT);
		$file = $this->_file_get($args['id']);
		$parent = _sql_single('SELECT content_id FROM daportal_download'
				." WHERE download_id='".$file['parent']."'");
		include('./modules/download/file_update.tpl');
	}


	//DownloadModule::system
	public function system($args)
	{
		global $html, $title, $error;

		$title.=' - '.DOWNLOADS;
		if(!isset($args['action']))
			return;
		if($_SERVER['REQUEST_METHOD'] == 'GET')
		{
			if($args['action'] == 'download')
				$html = 0;
			return;
		}
		if($_SERVER['REQUEST_METHOD'] != 'POST')
			return;
		if($args['action'] == 'config_update')
			$error = $this->_system_config_update($args);
		else if($args['action'] == 'file_insert')
			$html = 0;
		else if($args['action'] == 'file_update')
			$error = $this->_system_file_update($args);
	}

	private function _system_config_update($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		_config_update('download', $args);
		header('Location: '._module_link('download', 'admin'));
		exit(0);
	}

	private function _system_file_update($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		if(!isset($args['id']) || !isset($args['file'])
				|| !isset($args['comment']))
			return INVALID_ARGUMENT;
		require_once('./system/content.php');
		if(_content_update($args['id'], $args['file'], $args['comment'])
				== FALSE)
			return 'Unable to modify file';
		header('Location: '._module_link('download', FALSE,
					$args['id']));
		exit(0);
	}
}

?>
