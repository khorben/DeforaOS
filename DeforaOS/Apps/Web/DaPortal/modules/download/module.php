<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['BACK'] = 'Back';
$text['CREATE'] = 'Create';
$text['DOWNLOADS_ADMINISTRATION'] = 'Downloads administration';
$text['DOWNLOADS_LIST'] = 'Downloads list';
$text['FORWARD'] = 'Forward';
$text['IMAGE_PREVIEW'] = 'Image preview';
$text['MODE'] = 'Permissions';
$text['NEW_DIRECTORY'] = 'New directory';
$text['OWNER'] = 'Owner';
$text['PARENT_DIRECTORY'] = 'Parent directory';
$text['SETTINGS'] = 'Settings';
$text['UPLOAD_FILE'] = 'Upload file';
global $lang;
if($lang == 'fr')
{
	$text['BACK'] = 'Précédent';
	$text['CREATE'] = 'Créer';
	$text['DOWNLOADS_ADMINISTRATION'] = 'Administration des downloads';
	$text['DOWNLOADS_LIST'] = 'Liste des downloads';
	$text['FORWARD'] = 'Suivant';
	$text['NEW_DIRECTORY'] = 'Nouveau répertoire';
	$text['OWNER'] = 'Propriétaire';
	$text['PARENT_DIRECTORY'] = 'Répertoire parent';
	$text['SETTINGS'] = 'Paramètres';
}
_lang($text);
define('S_IFDIR', 01000);


function _permissions($mode)
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


function download_admin($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title download">'._html_safe(DOWNLOADS_ADMINISTRATION)
			.'</h1>'."\n".'<h2 class="title settings">'
			._html_safe(SETTINGS).'</h2>'."\n");
	if(($configs = _config_list('download')))
	{
		$module = 'download';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title download">'._html_safe(DOWNLOADS_LIST)
			."</h2>\n");
	$dls = _sql_array('SELECT daportal_content.content_id AS id'
			.', title AS name, daportal_content.enabled AS enabled'
			.', username AS owner, mode'
			.' FROM daportal_download, daportal_content'
			.', daportal_user WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' ORDER BY id DESC');
	if(!is_array($dls))
		return _error('Unable to list downloads');
	for($cnt = count($dls), $i = 0; $i < $cnt; $i++)
	{
		$dls[$i]['icon'] = $dls[$i]['mode'] & S_IFDIR
			? 'folder.png' : 'default.png';
		$dls[$i]['thumbnail'] = 'icons/48x48/mime/'.$dls[$i]['icon'];
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
			.($dls[$i]['enabled'] == 'enabled' ? ENABLED : DISABLED)
			.'"/>';
		$dls[$i]['owner'] = _html_safe($dls[$i]['owner']);
		$dls[$i]['mode'] = _html_safe(_permissions($dls[$i]['mode']));
	}
	$toolbar = array();
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => 'javascript:location.reload()'); /* XXX */
	_module('explorer', 'browse_trusted', array('entries' => $dls,
				'class' => array('enabled' => ENABLED,
					'owner' => OWNER, 'mode' => MODE),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'download', 'action' => 'admin'));
}


function download_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return download_admin(array());
}


function download_default($args)
{
	global $user_id;

	$parent = ' IS NULL';
	$sql = 'SELECT daportal_content.content_id AS id, download_id'
		.', title AS name, mode, parent, timestamp AS ctime, content'
		.', daportal_content.user_id AS user_id, username AS user'
		.' FROM daportal_download, daportal_content, daportal_user'
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
				return _default_download($file);
			$parent = "='".$file['download_id']."'";
		}
	}
	if(!isset($file) || !is_array($file))
		$file = array('id' => '');
	print('<h1 class="title download">'._html_safe(DOWNLOADS));
	if(isset($file['name']))
		print(': '._html_safe($file['name']));
	print("</h1>\n");
	$sql = 'SELECT daportal_content.content_id AS id, title AS name'
		.', daportal_content.enabled AS enabled, mode'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_download, daportal_content, daportal_user'
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
		$mime = _mime_from_ext($dls[$i]['name']);
		if($dls[$i]['mode'] & S_IFDIR)
		{
			$dls[$i]['icon'] = 'icons/16x16/mime/folder.png';
			$dls[$i]['thumbnail'] = 'icons/48x48/mime/folder.png';
			continue;
		}
		$dls[$i]['thumbnail'] = is_readable('icons/48x48/mime/'.$mime
				.'.png') ? 'icons/48x48/mime/'.$mime.'.png'
				: 'icons/48x48/mime/default.png';
		$dls[$i]['icon'] = is_readable('icons/16x16/mime/'.$mime.'.png')
			? 'icons/16x16/mime/'.$mime.'.png'
			: $dls[$i]['thumbnail'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => BACK, 'class' => 'back',
			'link' => 'javascript:history.back()'); /* XXX */
	$toolbar[] = array('title' => PARENT_DIRECTORY,
			'class' => 'parent_directory',
			'link' => _module_link('download', FALSE, FALSE, FALSE,
				isset($file['parent'])
				 ? 'download_id='.$file['parent'] : ''));
	$toolbar[] = array('title' => FORWARD, 'class' => 'forward',
			'link' => 'javascript:history.forward()'); /* XXX */
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH, 'class' => 'refresh',
			'link' => 'javascript:location.reload()'); /* XXX */
	if(_user_admin($user_id))
	{
		$toolbar[] = array();
		$toolbar[] = array('title' => NEW_DIRECTORY,
				'class' => 'new_directory',
				'link' => _module_link('download',
					'directory_new', $file['id']));
		$toolbar[] = array('title' => UPLOAD_FILE,
				'class' => 'upload_file',
				'link' => _module_link('download', 'file_new',
					$file['id']));
		$toolbar[] = array('title' => DELETE, 'class' => 'delete',
				'action' => 'delete', 'confirm' => 'delete');
	}
	_module('explorer', 'browse', array('entries' => $dls,
				'class' => array('username' => AUTHOR),
				'toolbar' => $toolbar, 'view' => 'thumbnails',
				'module' => 'download', 'action' => 'default',
				'id' => $file['id']));
}

function _default_download($file)
{
	//FIXME replace default icon
	if(!($root = _config_get('download', 'root')))
		return _error('No root directory');
	require_once('./system/mime.php');
	if(($file['mime'] = _mime_from_ext($file['name'])) == 'default')
		$file['mime'] = 'text/plain';
	$filename = $root.'/'.$file['download_id'];
	$st = stat($filename);
	$file['mode'] = _permissions($file['mode']);
	$file['ctime'] = strftime(DATE_FORMAT, $st['ctime']);
	$file['mtime'] = strftime(DATE_FORMAT, $st['mtime']);
	$file['atime'] = strftime(DATE_FORMAT, $st['atime']);
	$file['size'] = round($st['size'] / 1024);
	$file['size'] = $file['size'] > 1024 ? round($file['size'] / 1024).' MB'
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
	_sql_query('DELETE FROM daportal_download'
			." WHERE download_id='".$file['download_id']."'");
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
	if(!($id = _sql_single('SELECT content_id FROM daportal_download'
			." WHERE content_id='".$args['id']."'")))
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


function download_download($args)
{
	if(!is_numeric($args['id']))
		return _error(INVALID_ARGUMENT); //FIXME 404
	if(!($root = _config_get('download', 'root')))
		return _error('Internal server error'); //FIXME 501
	$file = _sql_array('SELECT title AS name, download_id'
			.' FROM daportal_download, daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			." AND enabled='1'"
			." AND daportal_content.content_id='".$args['id']."'");
	if(!is_array($file) || count($file) != 1
			|| !is_numeric($file[0]['download_id']))
		return _error('File not found'); //FIXME 404
	$file = $file[0];
	$filename = $root.'/'.$file['download_id'];
	if(!is_readable($filename))
		return _error(PERMISSION_DENIED); //FIXME 403
	if(($fp = fopen($filename, 'r')) == FALSE)
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
	$attachment = in_array($mime, $client_mime) ? 'inline' : 'attachment';
	header('Content-Type: '.$mime);
	header('Content-Length: '.filesize($filename));
	header('Content-Disposition: '.$attachment.'; filename="'
			.addslashes($file['name']).'"');
	if(isset($_SERVER['HTTP_RANGE'])
			&& preg_match_all('/^bytes=([0-9]+)-$/',
				$_SERVER['HTTP_RANGE'], $offset))
	{
		$offset = $offset[1][0];
		if(@fseek($fp, $offset) == 0)
		{
			$range = isset($st) ? ($st['size']-1).'/'.$st['size']
				: '*/*';
			header('Content-Range: bytes '.$offset.'-'.$range);
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


function download_file_insert($args)
{
	global $user_id;

	//FIXME call this code on system, do html if error
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = $_FILES['file']['name'];
	if(!($root = _config_get('download', 'root')))
		return _error('No root directory');
	$parent = is_numeric($args['parent']) ? "'".$args['parent']."'"
		: 'NULL';
	//FIXME not twice the same filename in a directory
	require_once('./system/content.php');
	if(!($content_id = _content_insert($title, '', 1)))
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

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title download">'._html_safe(UPLOAD_FILE)."</h1>\n");
	if(isset($args['id']))
		$parent = _sql_single('SELECT download_id'
		.' FROM daportal_download WHERE content_id='
		."'".$args['id']."'");
	include('./modules/download/file_update.tpl');
}


function download_system($args)
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
		$error = _system_config_update($args);
	else if($args['action'] == 'file_insert')
		$html = 0;
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('download', $args);
	header('Location: '._module_link('download', 'admin'));
	exit(0);
}

?>
