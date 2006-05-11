<?php //modules/download/module.php
//FIXME
//- permissions are not implemented



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['DOWNLOADS'] = 'Downloads';
$text['DOWNLOADS_ADMINISTRATION'] = 'Downloads administration';
$text['MODE'] = 'Permissions';
$text['NEW_DIRECTORY'] = 'New directory';
$text['UPLOAD_FILE'] = 'Upload file';
global $lang;
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
	print('<h1><img src="modules/download/icon.png" alt=""/> '
			._html_safe(DOWNLOADS_ADMINISTRATION).'</h1>'."\n");
	if(($configs = _config_list('download')))
	{
		$module = 'download';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	$dls = _sql_array('SELECT download_id AS id, title AS name, enabled'
			.', mode'
			.' FROM daportal_download, daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id ORDER BY id DESC;');
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
		$dls[$i]['enabled'] = ($dls[$i]['enabled'] == 't')
			? 'enabled' : 'disabled';
		$dls[$i]['enabled'] = '<img src="icons/16x16/'
			.$dls[$i]['enabled'].'.png" alt="'
			.$dls[$i]['enabled'].'.png" title="'
			.($dls[$i]['enabled'] == 'enabled' ? ENABLED : DISABLED)
			.'"/>';
		$dls[$i]['mode'] = _html_safe(_permissions($dls[$i]['mode']));
	}
	$toolbar = array();
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable');
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE,
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete', 'confirm' => 'delete');
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Refresh',
			'icon' => 'icons/16x16/refresh.png',
			'link' => 'javascript:location.reload()');
	_module('explorer', 'browse_trusted', array('entries' => $dls,
				'class' => array('enabled' => ENABLED,
					'mode' => MODE),
				'toolbar' => $toolbar, 'view' => 'details',
				'module' => 'download', 'action' => 'admin'));
}


function download_config_update($args)
	//FIXME would benefit of a global design
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^download_([a-zA-Z_]+)$', $k, $regs))
			_config_set('download', $regs[1], $args[$k], 0);
	header('Location: index.php?module=download&action=admin');
	exit(0);
}


function download_default($args)
{
	global $user_id;

	$parent = ' IS NULL';
	if(isset($args['id']) && is_numeric($args['id']))
	{
		$file = _sql_array('SELECT download_id AS id, title AS name'
				.', mode, parent, timestamp AS ctime, content'
				.', daportal_content.user_id, username AS user'
				.' FROM daportal_download, daportal_content'
				.', daportal_user'
				.' WHERE daportal_download.content_id'
				.'=daportal_content.content_id'
				.' AND daportal_content.user_id'
				.'=daportal_user.user_id'
				." AND daportal_content.enabled='1'"
				." AND daportal_user.enabled='1'"
				." AND download_id='".$args['id']."';");
		if(is_array($file) && count($file) == 1)
		{
			$file = $file[0];
			if(!($file['mode'] & S_IFDIR))
				return _default_download($file);
			$parent = "='".$args['id']."'";
		}
	}
	print('<h1><img src="modules/download/icon.png" alt=""/> '
			._html_safe(DOWNLOADS));
	if(isset($file['name']))
		print(': '._html_safe($file['name']));
	print('</h1>'."\n");
	$dls = _sql_array('SELECT download_id AS id, title AS name, enabled'
			.', mode'
			.' FROM daportal_download, daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			." AND enabled='1' AND parent$parent ORDER BY name;");
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
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'Back', 'icon' => 'icons/16x16/back.png',
			'link' => 'javascript:history.back()');
	$toolbar[] = array('title' => 'Parent directory',
			'icon' => 'icons/16x16/updir.png',
			'link' => 'index.php?module=download&action=default&id='
			.(isset($file['parent']) ? $file['parent'] : ''));
	$toolbar[] = array('title' => 'Forward',
			'icon' => 'icons/16x16/forward.png',
			'link' => 'javascript:history.forward()');
	if(_user_admin($user_id))
	{
		$toolbar[] = array();
		$toolbar[] = array('title' => 'New directory',
				'icon' => 'icons/16x16/newdir.png',
				'link' => 'index.php?module=download'
				.'&action=directory_new'
				.'&id='.$file['id']);
		$toolbar[] = array('title' => 'Upload file',
				'icon' => 'icons/16x16/save.png',
				'link' => 'index.php?module=download'
				.'&action=file_new'
				.'&id='.$file['id']);
		$toolbar[] = array();
		$toolbar[] = array('title' => DELETE,
				'icon' => 'icons/16x16/delete.png',
				'action' => 'delete', 'confirm' => 'delete');
	}
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Refresh',
			'icon' => 'icons/16x16/refresh.png',
			'link' => 'javascript:location.reload()');
	_module('explorer', 'browse', array('entries' => $dls,
				'toolbar' => $toolbar, 'view' => 'thumbnails',
				'module' => 'download', 'action' => 'default',
				'id' => $file['id']));
}

function _default_download($file)
{
	if(!($root = _config_get('download', 'root')))
		return _error('No root directory');
	require_once('./system/mime.php');
	if(($file['mime'] = _mime_from_ext($file['name'])) == 'default')
		$file['mime'] = 'text/plain';
	$filename = $root.'/'.$file['id'];
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
	$file = _sql_array('SELECT content_id, mode'
			.' FROM daportal_download'
			." WHERE download_id='".$args['id']."';");
	if(!is_array($file) || count($file) != 1)
		return _error(INVALID_ARGUMENT);
	$file = $file[0];
	//FIXME if directory look for childs
	require_once('./system/content.php');
	_sql_query('DELETE FROM daportal_download'
			." WHERE download_id='".$args['id']."';");
	_content_delete($file['content_id']);
	if(!($file['mode'] & S_IFDIR))
		unlink($root.'/'.$args['id']);
}


function download_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($id = _sql_single('SELECT content_id FROM daportal_download'
			." WHERE download_id='".$args['id']."';")))
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
	$parent = is_numeric($args['parent']) ? "'".$args['parent']."'"
		: 'NULL';
	//FIXME not twice the same filename in a directory
	require_once('./system/content.php');
	if(!($id = _content_insert($args['title'], '', 1)))
		return _error('Unable to create directory');
	if(!_sql_query('INSERT INTO daportal_download'
			.' (content_id, parent, mode) VALUES'
			." ('$id', $parent, '".S_IFDIR."');"))
	{
		_content_delete($id);
		return _error('Unable to create directory');
	}
	download_default(array('id' => $args['parent']));
}


function download_directory_new($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/download/directory.png" alt=""/> '
			._html_safe(NEW_DIRECTORY).'</h1>'."\n");
	$parent = $args['id'];
	include('./modules/download/directory_update.tpl');
}


function download_download($args)
{
	if(!is_numeric($args['id']))
		return _error(INVALID_ARGUMENT); //FIXME 404
	if(!($root = _config_get('download', 'root')))
		return _error('No root directory'); //FIXME 501
	$filename = $root.'/'.$args['id'];
	if(!is_readable($filename))
		return _error(PERMISSION_DENIED); //FIXME 403
	$file = _sql_array('SELECT title AS name'
			.' FROM daportal_download, daportal_content'
			.' WHERE daportal_download.content_id'
			.'=daportal_content.content_id'
			." AND enabled='1' AND download_id='".$args['id']."';");
	if(!is_array($file) || count($file) != 1)
		return _error('Unable to download file'); //FIXME 404
	$file = $file[0];
	if(($fp = fopen($filename, 'r')) == FALSE)
		return _error('Unable to open file'); //FIXME ???
	require_once('./system/mime.php');
	if(($mime = _mime_from_ext($file['name'])) == 'default')
		$mime = 'text/plain';
	$client_mime = explode(',', $_SERVER['HTTP_ACCEPT']);
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
			." WHERE download_id='".$args['id']."';")))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	_content_enable($id);
}


function download_file_insert($args)
{
	global $user_id;

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
	if(!_sql_query('INSERT INTO daportal_download'
				.' (content_id, parent) VALUES'
				." ('$content_id', $parent);"))
	{
		_content_delete($content_id);
		return _error('Unable to upload file');
	}
	$id = _sql_id('daportal_download', 'download_id');
	if(!rename($_FILES['file']['tmp_name'], $root.'/'.$id))
	{
		_sql_query('DELETE FROM daportal_download'
				." WHERE download_id='$id';");
		_content_delete($content_id);
		return _error('Unable to rename file');
	}
	download_default(array('id' => $args['parent']));
}


function download_file_new($args)
{
	global $user_id;

	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/download/icon.png" alt=""/> '
			._html_safe(UPLOAD_FILE).'</h1>'."\n");
	$parent = $args['id'];
	include('./modules/download/file_update.tpl');
}


function download_system($args)
{
	global $html, $title;

	$title.=' - '.DOWNLOADS;
	if($args['action'] == 'config_update' || $args['action'] == 'download')
		$html = 0;
}

?>
