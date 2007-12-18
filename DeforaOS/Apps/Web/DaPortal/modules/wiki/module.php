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



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['ALIGN_CENTER'] = 'Align center';
$text['ALIGN_JUSTIFY'] = 'Align justify';
$text['ALIGN_LEFT'] = 'Align left';
$text['ALIGN_RIGHT'] = 'Align right';
$text['BOLD'] = 'Bold';
$text['COPY'] = 'Copy';
$text['CUT'] = 'Cut';
$text['DOCUMENT_NOT_VALID'] = 'Document not valid';
$text['FONT'] = 'Font';
$text['INSERT_HORIZONTAL_RULE'] = 'Insert horizontal rule';
$text['INSERT_IMAGE'] = 'Insert image';
$text['INSERT_LINK'] = 'Insert link';
$text['ITALIC'] = 'Italic';
$text['MODIFICATION_OF_WIKI_PAGE'] = 'Modification of wiki page';
$text['NEW_WIKI_PAGE'] = 'New wiki page';
$text['PASTE'] = 'Paste';
$text['REDO'] = 'Redo';
$text['REVISIONS'] = 'Revisions';
$text['SETTINGS'] = 'Settings';
$text['SIZE'] = 'Size';
$text['STRIKE'] = 'Strike';
$text['STYLE'] = 'Style';
$text['SUBSCRIPT'] = 'Subscript';
$text['SUPERSCRIPT'] = 'Superscript';
$text['UNDERLINE'] = 'Underline';
$text['UNDO'] = 'Undo';
$text['WIKI'] = 'Wiki';
$text['WIKI_ADMINISTRATION'] = 'Wiki administration';
$text['WIKI_LIST'] = 'Wiki list';
$text['WIKI_PAGE_PREVIEW'] = 'Wiki page preview';
global $lang;
if($lang == 'fr')
{
	include('./modules/wiki/lang.fr.php');
}
_lang($text);


//private
function _exec($cmd)
{
	$output = array();
	$ret = 1;
	_info($cmd);
	exec($cmd, $output, $ret);
	return $ret == 0 ? TRUE : FALSE;
}

function _get($id, $lock = FALSE, $revision = FALSE)
{
	require_once('./system/content.php');
	$wiki = _content_select($id);
	if(!is_array($wiki) || strpos('/', $wiki['title']) != FALSE)
		return _error(INVALID_ARGUMENT);
	if(($root = _root()) == FALSE
			|| !is_readable($root.'/RCS/'.$wiki['title'].',v'))
		return _error('Internal server error');
	$cmd = $lock ? 'co -l' : 'co'; //XXX probable race conditions
	if($revision != FALSE)
		$cmd.=' -r'.escapeshellarg($revision);
	if(_exec($cmd.' '.escapeshellarg($root.'/'.$wiki['title'])) == FALSE)
		return _error('Could not checkout page');
	if(($wiki['content'] = file_get_contents($root.'/'.$wiki['title']))
			== FALSE)
	{
		unlink($root.'/'.$wiki['title']);
		return _error('Could not read page');
	}
	unlink($root.'/'.$wiki['title']);
	if(!_validate($wiki['content']))
		return _error(DOCUMENT_NOT_VALID);
	return $wiki;
}

function _root()
{
	if(($root = _config_get('wiki', 'root')) == FALSE)
		return FALSE;
	if(is_link($root.'/RCS'))
		return FALSE;
	if(!is_dir($root.'/RCS') && mkdir($root.'/RCS') != TRUE)
		return FALSE;
	return $root;
}

function _validate($content)
{
	$content = str_replace(array('<br>', '<hr>', '&nbsp;'),
			array('<br/>', '<hr/>', '&amp;nbsp;'), $content);
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$content = '<div>'.$content.'</div>';
	$parser = xml_parser_create(); //FIXME check encoding
	if(xml_set_element_handler($parser, '_validate_element',
				'_validate_element') != TRUE)
	{
		xml_parser_free($parser);
		return FALSE;
	}
	$_SESSION['wiki_script'] = 0;
	$ret = xml_parse($parser, $content);
	xml_parser_free($parser);
	if($_SESSION['wiki_script'] != 0)
	{
		unset($_SESSION['wiki_script']);
		return FALSE;
	}
	unset($_SESSION['wiki_script']);
	return $ret == 1 ? TRUE : FALSE;
}

function _validate_element($parser, $name, $attribs = FALSE)
{
	_info($name);
	if(strcasecmp($name, 'script') == 0)
		$_SESSION['wiki_script'] = 1;
}


//public
function wiki_admin($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title wiki">'._html_safe(WIKI_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('wiki')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'wiki';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title settings">'._html_safe(WIKI_LIST)."</h2>\n");
	$res = _sql_array('SELECT content_id AS id, title'
			.', daportal_content.enabled AS enabled, username'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id=daportal_user.user_id'
			." AND module_id='$module_id'");
	if(!is_array($res))
		return _error('Could not list wiki pages');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'wiki';
		$res[$i]['action'] = 'display';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['enabled'] = ($res[$i]['enabled'] == SQL_TRUE)
			? YES : NO;
	}
	_module('explorer', 'browse', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'username' => AUTHOR),
				'view' => 'details'));
}


function wiki_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return wiki_admin(array());
}


function wiki_default($args)
{
	//FIXME implement user_id

	if(isset($args['id']))
		return wiki_display($args);
	return wiki_list($args);
}


function wiki_display($args)
{
	$wiki = _get($args['id'], FALSE, isset($args['revision'])
			? $args['revision'] : FALSE);
	if(!is_array($wiki))
		return;
	$title = WIKI.': '.$wiki['title'];
	include('./modules/wiki/display.tpl');
	print('<h2 class="title users">'._html_safe(REVISIONS)."</h2");
	exec('rlog '.escapeshellarg(_root().'/'.$wiki['title']), $rcs);
	for($i = 0, $cnt = count($rcs); $i < $cnt;)
		if($rcs[$i++] == '----------------------------')
			break;
	$revisions = array();
	for(; $i < $cnt; $i+=3)
	{
		$name = _html_safe(substr($rcs[$i], 9));
		$date = _html_safe(substr($rcs[$i+1], 6, 19));
		$message = $rcs[$i+2];
		if($message == '----------------------------'
				|| $message ==
'=============================================================================')
			$message = '';
		else
		{
			$apnd = '';
			for($i++; $i < $cnt && $rcs[$i+2] !=
					'----------------------------'
					&& $rcs[$i+2] !=
'=============================================================================';
					$i++)
				$apnd = '...';
			$message.=$apnd;
			$username = ($user_id =_user_id($message)) != FALSE
				? '<a href="'._html_link('user', FALSE,
				$user_id, $message).'">'._html_safe($message)
					.'</a>' : '';
		}
		$revisions[] = array('module' => 'wiki', 'action' => 'display',
				'id' => $wiki['id'], 'name' => $name,
				'title' => $wiki['title'], 'date' => $date,
				'username' => $username,
				'args' => 'revision='.$name);
	}
	_module('explorer', 'browse_trusted', array('entries' => $revisions,
				'class' => array('date' => DATE,
					'username' => USERNAME),
				'view' => 'details'));
}


function wiki_insert($args)
{
	global $error;

	if(isset($error) && strlen($error))
		return _error($error);
	$title = NEW_WIKI_PAGE;
	if(isset($args['preview']) && isset($args['content']))
	{
		$title = WIKI_PAGE_PREVIEW;
		$wiki = array('title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']));
		if(!_validate($wiki['content']))
			_error(DOCUMENT_NOT_VALID);
		else
		{
			include('./modules/wiki/display.tpl');
			unset($title);
		}
	}
	include('./modules/wiki/update.tpl');
}


function wiki_list($args)
{
	global $module_id;

	print('<h1 class="title wiki">'._html_safe(WIKI_LIST)."</h1>\n");
	$sql = 'SELECT content_id AS id, title, username'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user'
		.' WHERE daportal_content.user_id=daportal_user.user_id'
		." AND module_id='$module_id'"
		." AND daportal_content.enabled='1'"
		." AND daportal_user.enabled='1'";
	$wiki = _sql_array($sql);
	if(!is_array($wiki))
		return _error('Could not list wiki pages');
	for($i = 0, $cnt = count($wiki); $i < $cnt; $i++)
	{
		$wiki[$i]['icon'] = 'icons/16x16/wiki.png';
		$wiki[$i]['thumbnail'] = 'icons/48x48/wiki.png';
		$wiki[$i]['module'] = 'wiki';
		$wiki[$i]['action'] = 'display';
		$wiki[$i]['name'] = $wiki[$i]['title'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_WIKI_PAGE, 'class' => 'new',
			'link' => _module_link('wiki', 'new'));
	_module('explorer', 'browse', array('entries' => $wiki,
				'class' => array('username' => AUTHOR),
				'toolbar' => $toolbar));
}


function wiki_modify($args)
{
	$wiki = _get($args['id']);
	if(!is_array($wiki))
		return;
	$title = MODIFICATION_OF_WIKI_PAGE.': '.$wiki['title'];
	include('./modules/wiki/update.tpl');
}


function wiki_new($args)
{
	$title = NEW_WIKI_PAGE;
	include('./modules/wiki/update.tpl');
}


function wiki_system($args)
{
	global $title;

	$title.=' - '.WIKI;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'config_update':
			return _system_config_update($args);
		case 'insert':
			return _system_insert($args);
		case 'update':
			return _system_update($args);
	}
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('wiki', $args);
	header('Location: '._module_link('wiki', 'admin'));
	exit(0);
}

function _system_insert($args)
{
	global $error, $user_id, $user_name;

	if($user_id == 0)
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _root()) == FALSE)
	{
		$error = 'Internal server error';
		return;
	}
	$title = stripslashes($args['title']);
	if(strlen($title) == 0 || strpos('/', $title) != FALSE
			|| $title == 'RCS')
	{
		$error = INVALID_ARGUMENT;
		return;
	}
	$content = stripslashes($args['content']);
	if(_validate($content) == FALSE)
	{
		$error = DOCUMENT_NOT_VALID;
		return;
	}
	if(($id = _sql_single('SELECT content_id FROM daportal_content'
					." WHERE title='".$args['title']."'"))
			!= FALSE)
	{
		$error = 'Title already exists';
		return;
	}
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], 'HTML content', 1))
			== FALSE) //FIXME content should have a type
	{
		$error = 'Could not insert content';
		return;
	}
	$filename = $root.'/'.$title;
	if(file_exists($filename) || file_exists($root.'/RCS/'.$title.',v'))
	{
		_content_delete($id);
		$error = 'Page already exists';
		return;
	}
	if(($fp = fopen($filename, 'w')) == FALSE)
	{
		_content_delete($id);
		$error = 'Could not write to page';
		return;
	}
	if(fwrite($fp, $content) == FALSE)
	{
		_content_delete($id);
		fclose($fp);
		$error = 'An error occured while writing';
		return;
	}
	fclose($fp);
	if(_exec('ci -u -m"'.escapeshellarg($user_name).'" '
				.escapeshellarg($filename)) == FALSE)
	{
		_content_delete($id);
		$error = 'An error occured while checking in';
		return;
	}
	header('Location: '._module_link('wiki', 'display', $id, $title));
	exit(0);
}

function _system_update($args)
{
	global $error, $user_id, $user_name;

	if($user_id == 0)
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _root()) == FALSE)
	{
		$error = 'Internal server error';
		return;
	}
	$wiki = _get($args['id'], TRUE);
	if(!is_array($wiki) || strpos('/', $wiki['title']) != FALSE)
	{
		$error = INVALID_ARGUMENT;
		return;
	}
	$id = $wiki['id'];
	$title = $wiki['title'];
	$content = stripslashes($args['content']);
	if(!_validate($content))
	{
		$error = DOCUMENT_NOT_VALID;
		return;
	}
	if(!file_exists($root.'/RCS/'.$title.',v'))
	{
		$error = 'Internal server error';
		return;
	}
	$filename = $root.'/'.$title;
	if(file_exists($filename) && unlink($filename) != TRUE)
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(($fp = fopen($filename, 'w')) == FALSE)
	{
		$error = 'Could not write to page';
		return;
	}
	if(fwrite($fp, $content) == FALSE)
	{
		fclose($fp);
		$error = 'An error occured while writing';
		return;
	}
	fclose($fp);
	if(_exec('ci -u -m'.escapeshellarg($user_name).' '
				.escapeshellarg($filename)) == FALSE)
	{
		unlink($filename);
		$error = 'An error occured while checking in';
		return;
	}
	unlink($filename);
	//insert plain text into database
	//FIXME factorize code and validation
	$_SESSION['wiki_content'] = '';
	$_SESSION['wiki_script'] = 0;
	$content = str_replace(array('<br>', '<hr>'), array('<br/>', '<hr/>'),
			$content);
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$parser = xml_parser_create();
	xml_set_character_data_handler($parser, '_update_data');
	if(xml_parse($parser, '<div>'.$content.'</div>') == 1)
		_content_update($id, FALSE, $_SESSION['wiki_content']);
	xml_parser_free($parser);
	unset($_SESSION['wiki_content']);
	header('Location: '._module_link('wiki', 'display', $id, $title));
	exit(0);
}

function _update_data($parser, $data)
{
	$_SESSION['wiki_content'].="\n".addslashes($data);
}


function wiki_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		return _error($error);
	$wiki = _get($args['id']);
	if(!is_array($wiki))
		return _error(INVALID_ARGUMENT);
	$wiki['content'] = stripslashes($args['content']);
	if(!_validate($wiki['content']))
		return _error(DOCUMENT_NOT_VALID);
	$title = WIKI_PAGE_PREVIEW.': '.$wiki['title'];
	if(isset($args['preview']))
		include('./modules/wiki/display.tpl');
	$title = MODIFICATION_OF_WIKI_PAGE.': '.$wiki['title'];
	include('./modules/wiki/update.tpl');
}


?>
