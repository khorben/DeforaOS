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
$text['BULLETS'] = 'Bullets';
$text['COPY'] = 'Copy';
$text['CUT'] = 'Cut';
$text['DOCUMENT_NOT_VALID'] = 'Document not valid';
$text['ENUMERATED'] = 'Enumerated';
$text['FONT'] = 'Font';
$text['INDENT'] = 'Indent';
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
$text['UNINDENT'] = 'Unindent';
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
//functions
function _wiki_exec($cmd)
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

function _wiki_get($id, $lock = FALSE, $revision = FALSE)
{
	require_once('./system/content.php');
	$wiki = _content_select($id);
	if(!is_array($wiki) || strpos('/', $wiki['tag']) != FALSE)
		return _error(INVALID_ARGUMENT);
	if(($root = _wiki_root()) == FALSE
			|| !is_readable($root.'/RCS/'.$wiki['tag'].',v'))
		return _error('Internal server error');
	$cmd = $lock ? 'co -l' : 'co'; //XXX probable race conditions
	if($revision != FALSE)
		$cmd.=' -r'.escapeshellarg($revision);
	if(_wiki_exec($cmd.' '.escapeshellarg($root.'/'.$wiki['tag'])) == FALSE)
		return _error('Could not checkout page');
	if(($wiki['content'] = file_get_contents($root.'/'.$wiki['tag']))
			=== FALSE)
	{
		unlink($root.'/'.$wiki['tag']);
		return _error('Could not read page');
	}
	unlink($root.'/'.$wiki['tag']);
	require_once('./system/xml.php');
	if(!_xml_validate($wiki['content'], $message))
		return _error(DOCUMENT_NOT_VALID.": $message");
	return $wiki;
}

function _wiki_root()
{
	if(($root = _config_get('wiki', 'root')) == FALSE)
		return FALSE;
	if(is_link($root.'/RCS'))
		return FALSE;
	if(!is_dir($root.'/RCS') && mkdir($root.'/RCS') != TRUE)
		return FALSE;
	return $root;
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
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id=daportal_user.user_id'
			." AND module_id='$module_id' ORDER BY title ASC");
	if(!is_array($res))
		return _error('Could not list wiki pages');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/wiki.png';
		$res[$i]['thumbnail'] = 'icons/48x48/wiki.png';
		$res[$i]['module'] = 'wiki';
		$res[$i]['apply_module'] = 'wiki';
		$res[$i]['action'] = 'display';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['username'] = '<a href="'._html_link('user', FALSE,
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_WIKI_PAGE, 'class' => 'new',
			'link' => _module_link('wiki', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'username' => AUTHOR),
				'module' => 'wiki', 'action' => 'admin',
				'toolbar' => $toolbar, 'view' => 'details'));
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
			$res[$i]['date'] = strftime('%d/%m/%Y %H:%M:%S',
					strtotime($res[$i]['date']));
			$res[$i]['date'] = _html_safe($res[$i]['date']);
			$res[$i]['content'] = str_replace("\n", ' ',
					substr($res[$i]['content'], 0, 40))
				.'...';
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
		$res[$i]['tag'] = $res[$i]['title'];
	}
	_module('explorer', 'browse', array('entries' => $res,
				'view' => 'details', 'toolbar' => 0));
}


//wiki_delete
function wiki_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	$res = _content_select($args['id']);
	if(!is_array($res))
		return _error(INVALID_ARGUMENT);
	if(($root = _wiki_root()) == FALSE)
		return 'Internal server error';
	if(strlen($res['title']) == 0 || strpos('/', $res['title']) !== FALSE
			|| $res['title'] == 'RCS')
		return _error(INVALID_ARGUMENT);
	@unlink($root.'/'.$res['title']); /* we can ignore this error */
	if(unlink($root.'/RCS/'.$res['title'].',v') != TRUE)
		return _error(INTERNAL_SERVER_ERROR);
	if(!_content_delete($args['id']))
		return _error('Could not delete wiki page');
}


//wiki_disable
function wiki_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable wiki page');
}


function wiki_display($args)
{
	$wiki = _wiki_get($args['id'], FALSE, isset($args['revision'])
			? $args['revision'] : FALSE);
	if(!is_array($wiki))
		return;
	$title = WIKI.': '.$wiki['title'];
	include('./modules/wiki/display.tpl');
	print('<h2 class="title users">'._html_safe(REVISIONS)."</h2>\n");
	exec('rlog '.escapeshellarg(_wiki_root().'/'.$wiki['title']), $rcs);
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


//wiki_enable
function wiki_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable wiki page');
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
		require_once('./system/xml.php');
		if(!_xml_validate($wiki['content'], $message))
			_error(DOCUMENT_NOT_VALID.": $message");
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
		$wiki[$i]['date'] = strftime('%d/%m/%Y %H:%M:%S',
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
	$wiki = _wiki_get($args['id']);
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
	if(($root = _wiki_root()) == FALSE)
		return 'Internal server error';
	if(!isset($args['title']))
		return INVALID_ARGUMENT;
	$title = stripslashes($args['title']);
	if(strlen($title) == 0 || strpos('/', $title) !== FALSE
			|| $title == 'RCS')
		return INVALID_ARGUMENT;
	$content = stripslashes($args['content']);
	require_once('./system/xml.php');
	if(_xml_validate($content, $message) == FALSE)
		return DOCUMENT_NOT_VALID.": $message";
	$sql = 'SELECT content_id FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND daportal_module.name='wiki'"
		." AND title='".$args['title']."'";
	if(($id = _sql_single($sql)) != FALSE)
		return 'Title already exists';
	require_once('./system/xml.php');
	if(($text = _xml_text($content, $message)) === FALSE)
	{
		_error($message, 0);
		$text = 'HTML content';
	}
	$text = addslashes($text);
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $text, 1)) == FALSE)
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
	if(fwrite($fp, $content) === FALSE)
	{
		_content_delete($id);
		fclose($fp);
		unlink($filename);
		return 'An error occured while writing';
	}
	fclose($fp);
	if(_wiki_exec('ci -u -m'.escapeshellarg($user_name).' '
				.escapeshellarg($filename)) == FALSE)
	{
		_content_delete($id);
		unlink($filename);
		return 'An error occured while checking in';
	}
	header('Location: '._module_link('wiki', FALSE, $id, $title));
	exit(0);
}

function _wiki_system_update($args)
{
	global $user_id, $user_name, $wiki_content;

	if($user_id == 0 && _config_get('wiki', 'anonymous') != TRUE)
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	if(($root = _wiki_root()) == FALSE)
		return 'Internal server error';
	$wiki = _wiki_get($args['id'], TRUE);
	if(!is_array($wiki) || strpos('/', $wiki['title']) != FALSE)
		return INVALID_ARGUMENT;
	$id = $wiki['id'];
	$title = $wiki['title'];
	$content = stripslashes($args['content']);
	require_once('./system/xml.php');
	if(!_xml_validate($content, $message))
		return DOCUMENT_NOT_VALID.": $message";
	if(!file_exists($root.'/RCS/'.$title.',v'))
		return 'Internal server error';
	$filename = $root.'/'.$title;
	if(file_exists($filename) && unlink($filename) != TRUE)
		return PERMISSION_DENIED;
	if(($fp = fopen($filename, 'w')) == FALSE)
		return 'Could not write to page';
	if(fwrite($fp, $content) === FALSE)
	{
		fclose($fp);
		return 'An error occured while writing';
	}
	fclose($fp);
	if(_wiki_exec('ci -u -m'.escapeshellarg($user_name).' '
				.escapeshellarg($filename)) == FALSE)
	{
		unlink($filename);
		return 'An error occured while checking in';
	}
	unlink($filename);
	//insert plain text into database
	require_once('./system/xml.php');
	if(($content = _xml_text($content)) !== FALSE)
	{
		_content_update($id, FALSE, addslashes($content),
				date('Y-m-d H:i:s'));
		_content_set_user($id, $user_id);
	}
	header('Location: '._module_link('wiki', FALSE, $id, $title));
	exit(0);
}


function wiki_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		return _error($error);
	$wiki = _wiki_get($args['id']);
	if(!is_array($wiki))
		return _error(INVALID_ARGUMENT);
	$wiki['content'] = stripslashes($args['content']);
	require_once('./system/xml.php');
	if(!_xml_validate($wiki['content'], $message))
		return _error(DOCUMENT_NOT_VALID.": $message");
	$title = WIKI_PAGE_PREVIEW.': '.$wiki['title'];
	if(isset($args['preview']))
	{
		include('./modules/wiki/display.tpl');
	}
	$title = MODIFICATION_OF_WIKI_PAGE.': '.$wiki['title'];
	include('./modules/wiki/update.tpl');
}


?>
