<?php //$Id$
//Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>
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
$text['LOOK_FOR_A_PAGE'] = 'Look for a page';
$text['MODIFICATION_OF_WIKI_PAGE'] = 'Modification of wiki page';
$text['NEW_WIKI_PAGE'] = 'New wiki page';
$text['PASTE'] = 'Paste';
$text['PREVIEW'] = 'Preview';
$text['RECENT_CHANGES'] = 'Recent changes';
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
$text['WIKI_PAGE_PREVIEW'] = 'Wiki page preview';
$text['WIKI_PAGES_LIST'] = 'Wiki pages list';
global $lang;
if($lang == 'fr')
{
	include('./modules/wiki/lang.fr.php');
}
_lang($text);


//private
//variables
global $wiki_blacklisted, $wiki_attrib_whitelist, $wiki_tag_whitelist,
       $wiki_content;
$wiki_blacklisted = 1;
$wiki_attrib_whitelist = array('alt', 'border', 'class', 'colspan', 'height',
		'href', 'size', 'src', 'style', 'title', 'width');
$wiki_tag_whitelist = array('a', 'acronym', 'b', 'big', 'br', 'center', 'div',
		'font', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
		'hr', 'i', 'img', 'li', 'ol', 'p', 'pre', 'span', 'sub', 'sup',
		'table', 'tbody', 'td', 'th', 'tr', 'tt', 'u', 'ul');
$wiki_content = '';


//functions
function _exec($cmd)
{
	$output = array();
	$ret = 1;
	exec($cmd, $output, $ret);
	_info("Command \"$cmd\" returned $ret");
	$i = 1;
	foreach($output as $o)
		_info($i++.": $o");
	return $ret == 0 ? TRUE : FALSE;
}

function _get($id, $lock = FALSE, $revision = FALSE)
{
	require_once('./system/content.php');
	$wiki = _content_select($id);
	if(!is_array($wiki) || strpos('/', $wiki['tag']) != FALSE)
		return _error(INVALID_ARGUMENT);
	if(($root = _root()) == FALSE
			|| !is_readable($root.'/RCS/'.$wiki['tag'].',v'))
		return _error('Internal server error');
	$cmd = $lock ? 'co -l' : 'co'; //XXX probable race conditions
	if($revision != FALSE)
		$cmd.=' -r'.escapeshellarg($revision);
	if(_exec($cmd.' '.escapeshellarg($root.'/'.$wiki['tag'])) == FALSE)
		return _error('Could not checkout page');
	if(($wiki['content'] = file_get_contents($root.'/'.$wiki['tag']))
			== FALSE)
	{
		unlink($root.'/'.$wiki['tag']);
		return _error('Could not read page');
	}
	unlink($root.'/'.$wiki['tag']);
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
	global $wiki_blacklisted;

	$content = str_replace(array('<br>', '<hr>', '&copy;', '&laquo;',
				'&nbsp;', '&raquo;'),
			array('<br/>', '<hr/>', '&amp;copy;', '&amp;laquo;',
				'&amp;nbsp;', '&amp;raquo;'),
			$content);
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$content = '<div>'.$content.'</div>';
	$parser = xml_parser_create(); //FIXME check encoding
	if(xml_set_element_handler($parser, '_validate_element_start',
				'_validate_element_end') != TRUE)
	{
		xml_parser_free($parser);
		return FALSE;
	}
	$wiki_blacklisted = 0;
	if(($ret = xml_parse($parser, $content)) != 1)
		$error = xml_error_string(xml_get_error_code($parser));
	xml_parser_free($parser);
	if($wiki_blacklisted != 0)
		return FALSE;
	$wiki_blacklisted = 1;
	if($ret == 1)
		return TRUE;
	_error('XML error: '.$error, 0);
	return FALSE;
}

function _validate_element_start($parser, $name, $attribs)
{
	global $wiki_blacklisted, $wiki_attrib_whitelist, $wiki_tag_whitelist;

	//return immediately if already detected as invalid
	//FIXME disable check in parser instead if possible
	if($wiki_blacklisted != 0)
		return;
	//check the element name
	$wcnt = count($wiki_tag_whitelist);
	for($i = 0; $i < $wcnt; $i++)
		if(strcasecmp($name, $wiki_tag_whitelist[$i]) == 0)
			break;
	if($i == $wcnt) //tag not found
	{
		$wiki_blacklisted = 1;
		_error('The tag "'.$name.'" is forbidden', 0);
		return;
	}
	//check every attribute
	$keys = array_keys($attribs);
	$attr = array();
	$i = 0;
	foreach($keys as $k)
		$attr[$i++] = $k;
	$acnt = count($attr);
	$wcnt = count($wiki_attrib_whitelist);
	for($i = 0; $i < $acnt; $i++)
	{
		$a = $attr[$i];
		for($j = 0; $j < $wcnt; $j++)
			if(strcasecmp($a, $wiki_attrib_whitelist[$j]) == 0)
				break;
		if($j == $wcnt) //attrib not found
			break;
	}
	if($i == $acnt)
		return;
	_error('The attribute "'.$a.'" is forbidden', 0);
	$wiki_blacklisted = 1;
}

function _validate_element_end($parser, $name)
{
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
	print('<h2 class="title wiki">'._html_safe(WIKI_PAGES_LIST)."</h2>\n");
	$res = _sql_array('SELECT content_id AS id, title'
			.', daportal_content.enabled AS enabled, username'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id=daportal_user.user_id'
			." AND module_id='$module_id'");
	if(!is_array($res))
		return _error('Could not list wiki pages');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/wiki.png';
		$res[$i]['thumbnail'] = 'icons/48x48/wiki.png';
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
	//FIXME factorize code

	if(isset($args['id']))
		return wiki_display($args);
	if(isset($args['user_id']))
		return wiki_list($args);
	if(!isset($args['title']) || strlen($args['title']) == 0)
	{
		print('<h1 class="title wiki">'._html_safe(WIKI)."</h1>\n");
		include('./modules/wiki/default.tpl');
		print('<h2 class="title wiki">'._html_safe(RECENT_CHANGES)
				."</h2>\n");
		$sql = 'SELECT content_id AS id, timestamp AS date'
			.', name AS module, title'
			.', daportal_user.user_id AS user_id, username'
			.', content FROM daportal_content, daportal_module'
			.', daportal_user WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_module.name='wiki'"
			." AND daportal_content.enabled='1'"
			.' ORDER BY timestamp DESC '._sql_offset(0, 10);
		$res = _sql_array($sql);
		if(!is_array($res))
			return _error('Could not list recent changes');
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$res[$i]['icon'] = 'icons/16x16/wiki.png';
			$res[$i]['action'] = 'display';
			$res[$i]['name'] = $res[$i]['title'];
			$res[$i]['date'] = substr($res[$i]['date'], 0, 19);
			$res[$i]['content'] = str_replace("\n", ' ',
					substr($res[$i]['content'], 0, 40))
				.'...';
			$res[$i]['date'] = _html_safe($res[$i]['date']);
			$res[$i]['tag'] = $res[$i]['title'];
			$res[$i]['username'] = '<a href="'
				._html_link('user', FALSE, $res[$i]['user_id'],
						$res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
			$res[$i]['content'] = _html_safe($res[$i]['content']);
		}
		_module('explorer', 'browse_trusted', array('entries' => $res,
					'class' => array('date' => DATE,
						'username' => AUTHOR,
						'content' => PREVIEW),
					'view' => 'details', 'toolbar' => 0));
		return;
	}
	$title = stripslashes($args['title']);
	$sql = 'SELECT content_id AS id, name AS module, title, content'
		.' FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND daportal_module.name='wiki'"
		." AND daportal_content.enabled='1'"
		." AND daportal_content.title LIKE '%".$args['title']."%'";
	$res = _sql_array($sql);
	if(!is_array($res))
		_error(INVALID_ARGUMENT);
	else if(count($res) == 0)
	{
		print('<p>There was no match, but you can <a href="'
				._html_link('wiki', 'insert', FALSE,
					FALSE, array('title' => $title))
				.'">create the page</a>.</p>');
		return include('./modules/wiki/default.tpl');
	}
	if(count($res) == 1)
		return wiki_display(array('id' => $res[0]['id']));
	print('<h1 class="title wiki">'._html_safe(WIKI)."</h1>\n");
	include('./modules/wiki/default.tpl');
	_info('More than one page matched:', 1);
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/wiki.png';
		$res[$i]['action'] = 'display';
		$res[$i]['name'] = $res[$i]['title'];
	}
	_module('explorer', 'browse', array('entries' => $res,
				'view' => 'details', 'toolbar' => 0));
}


function wiki_display($args)
{
	$wiki = _get($args['id'], FALSE, isset($args['revision'])
			? $args['revision'] : FALSE);
	if(!is_array($wiki))
		return;
	$title = WIKI.': '.$wiki['title'];
	include('./modules/wiki/display.tpl');
	print('<h2 class="title users">'._html_safe(REVISIONS)."</h2>\n");
	exec('rlog '.escapeshellarg(_root().'/'.$wiki['title']), $rcs);
	for($i = 0, $cnt = count($rcs); $i < $cnt;)
		if($rcs[$i++] == '----------------------------')
			break;
	$revisions = array();
	for(; $i < $cnt - 2; $i+=3)
	{
		$name = _html_safe(substr($rcs[$i], 9));
		$date = _html_safe(substr($rcs[$i+1], 6, 19));
		$sep = '======================================================';
		$message = $rcs[$i+2];
		if($message == '----------------------------'
				|| strncmp($message, $sep, strlen($sep)) == 0)
			$message = '';
		else
		{
			$apnd = '';
			for($i++; $i < $cnt && $rcs[$i+2] !=
					'----------------------------'
					&& strncmp($rcs[$i+2], $sep,
						strlen($sep)) != 0; $i++)
				$apnd = '...';
			$message.=$apnd;
			require_once('./system/user.php');
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
					'username' => AUTHOR),
				'toolbar' => FALSE, 'view' => 'details'));
}


function wiki_insert($args)
{
	global $error, $user_id;

	if(isset($error) && strlen($error))
		return _error($error);
	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return _error(PERMISSION_DENIED);
	//FIXME check that the page doesn't already exist
	$title = NEW_WIKI_PAGE;
	$wiki = array();
	if(isset($args['title']))
		$wiki['title'] = stripslashes($args['title']);
	if(isset($args['preview']) && isset($args['content']))
	{
		$title = WIKI_PAGE_PREVIEW;
		$wiki['content'] = stripslashes($args['content']);
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

	$title = WIKI_PAGES_LIST;
	$classes = array('date' => DATE);
	$sql = 'SELECT content_id AS id, title, timestamp AS date'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user'
		.' WHERE daportal_content.user_id=daportal_user.user_id'
		." AND module_id='$module_id'"
		." AND daportal_content.enabled='1'"
		." AND daportal_user.enabled='1'";
	if(isset($args['user_id']))
	{
		require_once('./system/user.php');
		if(($username = _user_name($args['user_id'])) == FALSE)
			return _error(INVALID_ARGUMENT);
		$title.=_BY_.$username;
		$sql.=" AND daportal_content.user_id='".$args['user_id']."'";
	}
	else
		$classes['username'] = AUTHOR;
	print('<h1 class="title wiki">'._html_safe($title)."</h1>\n");
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
		$wiki[$i]['tag'] = $wiki[$i]['title'];
		$wiki[$i]['date'] = strftime('%d/%m/%y %H:%M:%S',
				strtotime(substr($wiki[$i]['date'], 0, 19)));
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_WIKI_PAGE, 'class' => 'new',
			'link' => _module_link('wiki', 'new'));
	_module('explorer', 'browse', array('entries' => $wiki,
				'class' => $classes, 'toolbar' => $toolbar,
				'view' => 'details'));
}


function wiki_modify($args)
{
	global $user_id;

	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return _error(PERMISSION_DENIED);
	$wiki = _get($args['id']);
	if(!is_array($wiki))
		return;
	$title = MODIFICATION_OF_WIKI_PAGE.': '.$wiki['title'];
	include('./modules/wiki/update.tpl');
}


function wiki_new($args)
{
	global $user_id;

	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return _error(PERMISSION_DENIED);
	$title = NEW_WIKI_PAGE;
	include('./modules/wiki/update.tpl');
}


function wiki_system($args)
{
	global $title, $error;

	$title.=' - '.WIKI;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'config_update':
			$error = _wiki_system_config_update($args);
			break;
		case 'insert':
			$error = _wiki_system_insert($args);
			break;
		case 'update':
			$error = _wiki_system_update($args);
			break;
	}
}

function _wiki_system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	$args['wiki_anonymous'] = isset($args['wiki_anonymous']) ? TRUE : FALSE;
	_config_update('wiki', $args);
	header('Location: '._module_link('wiki', 'admin'));
	exit(0);
}

function _wiki_system_insert($args)
{
	global $user_id, $user_name;

	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _root()) == FALSE)
		return 'Internal server error';
	$title = stripslashes($args['title']);
	if(strlen($title) == 0 || strpos('/', $title) != FALSE
			|| $title == 'RCS')
		return INVALID_ARGUMENT;
	$content = stripslashes($args['content']);
	if(_validate($content) == FALSE)
		return DOCUMENT_NOT_VALID;
	$sql = 'SELECT content_id FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND daportal_module.name='wiki'"
		." AND title='".$args['title']."'";
	if(($id = _sql_single($sql)) != FALSE)
		return 'Title already exists';
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], 'HTML content', 1))
			== FALSE) //FIXME insert plain text into database
		return 'Could not insert content';
	$filename = $root.'/'.$title;
	if(file_exists($filename) || file_exists($root.'/RCS/'.$title.',v'))
	{
		_content_delete($id);
		return 'Page already exists';
	}
	if(($fp = fopen($filename, 'w')) == FALSE)
	{
		_content_delete($id);
		return 'Could not write to page';
	}
	if(fwrite($fp, $content) == FALSE)
	{
		_content_delete($id);
		fclose($fp);
		return 'An error occured while writing';
	}
	fclose($fp);
	if(_exec('ci -u -m'.escapeshellarg($user_name).' '
				.escapeshellarg($filename)) == FALSE)
	{
		_content_delete($id);
		unlink($filename);
		return 'An error occured while checking in';
	}
	header('Location: '._module_link('wiki', 'display', $id, $title));
	exit(0);
}

function _wiki_system_update($args)
{
	global $user_id, $user_name, $wiki_content;

	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _root()) == FALSE)
		return 'Internal server error';
	$wiki = _get($args['id'], TRUE);
	if(!is_array($wiki) || strpos('/', $wiki['title']) != FALSE)
		return INVALID_ARGUMENT;
	$id = $wiki['id'];
	$title = $wiki['title'];
	$content = stripslashes($args['content']);
	if(!_validate($content))
		return DOCUMENT_NOT_VALID;
	if(!file_exists($root.'/RCS/'.$title.',v'))
		return 'Internal server error';
	$filename = $root.'/'.$title;
	if(file_exists($filename) && unlink($filename) != TRUE)
		return PERMISSION_DENIED;
	if(($fp = fopen($filename, 'w')) == FALSE)
		return 'Could not write to page';
	if(fwrite($fp, $content) == FALSE)
	{
		fclose($fp);
		return 'An error occured while writing';
	}
	fclose($fp);
	if(_exec('ci -u -m'.escapeshellarg($user_name).' '
				.escapeshellarg($filename)) == FALSE)
	{
		unlink($filename);
		return 'An error occured while checking in';
	}
	unlink($filename);
	//insert plain text into database
	//FIXME factorize code and validation
	$wiki_content = '';
	$content = str_replace(array('<br>', '<hr>'), array('<br/>', '<hr/>'),
			$content);
	$content = preg_replace('/(<img [^>]*)>/', '\1/>', $content);
	$parser = xml_parser_create();
	xml_set_character_data_handler($parser, '_update_data');
	if(xml_parse($parser, '<div>'.$content.'</div>') == 1)
		_content_update($id, FALSE, $wiki_content);
	xml_parser_free($parser);
	$wiki_content = '';
	header('Location: '._module_link('wiki', 'display', $id, $title));
	exit(0);
}

function _update_data($parser, $data)
{
	global $wiki_content;

	$wiki_content.="\n".addslashes($data);
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
