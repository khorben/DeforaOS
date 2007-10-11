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
$text['DOCUMENT_NOT_VALID'] = 'Document not valid';
$text['FONT'] = 'Font';
$text['MODIFICATION_OF_WIKI_PAGE'] = 'Modification of wiki page';
$text['NEW_WIKI_PAGE'] = 'New wiki page';
$text['REVISIONS'] = 'Revisions';
$text['SIZE'] = 'Size';
$text['STYLE'] = 'Style';
$text['WIKI'] = 'Wiki';
$text['WIKI_ADMINISTRATION'] = 'Wiki administration';
$text['WIKI_LIST'] = 'Wiki list';
$text['WIKI_PAGE_PREVIEW'] = 'Wiki page preview';
global $lang;
if($lang == 'fr')
{
	$text['DOCUMENT_NOT_VALID'] = 'Document non valide';
	$text['WIKI_ADMINISTRATION'] = 'Administration du wiki';
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
		$cmd.=' -r"'.escapeshellarg($revision).'"';
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
	$content = str_replace('<br>', '<br/>', $content);
	$content = '<div>'.$content.'</div>';
	$parser = xml_parser_create(); //FIXME check encoding
	$ret = xml_parse($parser, $content);
	xml_parser_free($parser);
	return $ret == 1 ? TRUE : FALSE;
}


//public
function wiki_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title wiki">'._html_safe(WIKI_ADMINISTRATION)
			."</h1>\n");
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
	$sql = 'SELECT content_id AS id, title AS name'
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
		$wiki[$i]['module'] = 'wiki';
		$wiki[$i]['action'] = 'display';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_WIKI_PAGE, 'class' => 'new',
			'link' => _module_link('wiki', 'new'));
	_module('explorer', 'browse', array('entries' => $wiki,
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
		case 'insert':
			return _system_insert($args);
		case 'update':
			return _system_update($args);
	}
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
					." WHERE title='".$args['title']
					."'")) != FALSE)
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
	if(_validate($content) == FALSE)
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
	if(_exec('ci -u -m"'.escapeshellarg($user_name)
				.'" '.escapeshellarg($filename)) == FALSE)
	{
		unlink($filename);
		$error = 'An error occured while checking in';
		return;
	}
	unlink($filename);
	header('Location: '._module_link('wiki', 'display', $id, $title));
	exit(0);
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
