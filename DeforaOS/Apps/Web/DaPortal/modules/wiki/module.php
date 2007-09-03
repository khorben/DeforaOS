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
$text['MODIFICATION_OF_WIKI_PAGE'] = 'Modification of wiki page';
$text['NEW_WIKI_PAGE'] = 'New wiki page';
$text['WIKI'] = 'Wiki';
$text['WIKI_ADMINISTRATION'] = 'Wiki administration';
$text['WIKI_LIST'] = 'Wiki list';
global $lang;
if($lang == 'fr')
	$text['WIKI_ADMINISTRATION'] = 'Administration du wiki';
_lang($text);


//private
function _get($id)
{
	global $module_id; //XXX

	$sql = 'SELECT content_id AS id, title FROM daportal_content'
		." WHERE module_id='$module_id'"
		." AND enabled='1' AND content_id='$id'";
	$wiki = _sql_array($sql);
	if(!is_array($wiki) || count($wiki) != 1
			|| strpos('/', $wiki[0]['title']) != FALSE)
		return _error(INVALID_ARGUMENT);
	$wiki = $wiki[0];
	if(($root = _config_get('wiki', 'root')) == FALSE
			|| !is_readable($root.'/RCS/'.$wiki['title'].',v'))
		return _error('Internal server error');
	$cmd = 'co '.escapeshellcmd($root.'/'.$wiki['title']);
	$output = array();
	$ret = 1;
	_info($cmd);
	exec($cmd, $output, $ret);
	if($ret != 0)
		return _error('Could not checkout page');
	if(($wiki['content'] = file_get_contents($root.'/'.$wiki['title']))
			== FALSE)
		return _error('Could not read page');
	if(!_validate($wiki['content']))
		return _error('Document not valid');
	return $wiki;
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
	if(isset($args['id']))
		return wiki_display($args);
	return wiki_list($args);
}


function wiki_display($args)
{
	$wiki = _get($args['id']);
	if(!is_array($wiki))
		return;
	$title = WIKI.': '.$wiki['title'];
	include('./modules/wiki/display.tpl');
}


function wiki_insert($args)
{
	global $error;

	if(isset($error) && strlen($error))
		return _error($error);
	$title = NEW_WIKI_PAGE;
	if(isset($args['preview']) && isset($args['content']))
		$wiki = array('content' => $args['content']);
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
	global $error, $user_id;

	if($user_id == 0)
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _config_get('wiki', 'root')) == FALSE
			|| is_link($root.'/RCS')
			|| (!is_dir($root.'/RCS')
				&& mkdir($root.'/RCS') != TRUE))
	{
		$error = 'Internal server error';
		return;
	}
	$title = stripslashes($args['title']);
	if(strpos('/', $title) != FALSE || $title == 'RCS')
	{
		$error = INVALID_ARGUMENT;
		return;
	}
	$content = stripslashes($args['content']);
	if(_validate(stripslashes($content)) == FALSE)
	{
		$error = 'Document not valid';
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
		$error = 'Page already exists';
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
	$cmd = 'ci '.escapeshellcmd($filename);
	$output = array();
	$ret = 1;
	_info($cmd);
	exec($cmd, $output, $ret);
	if($ret != 0)
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
}


?>
