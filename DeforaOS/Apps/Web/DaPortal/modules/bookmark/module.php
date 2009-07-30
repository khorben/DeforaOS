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
$text['ADDRESS'] = 'Address';
$text['BOOKMARK_LIST'] = 'Bookmark list';
$text['BOOKMARKS'] = 'Bookmarks';
$text['BOOKMARKS_ADMINISTRATION'] = 'Bookmarks administration';
$text['MODIFICATION_OF'] = 'Modification of';
$text['NEW_BOOKMARK'] = 'New bookmark';
$text['PRIVATE'] = 'Private';
$text['PUBLIC'] = 'Public';
global $lang;
if($lang == 'de')
	$text['PRIVATE'] = 'Privat';
else if($lang == 'fr')
{
	$text['ADDRESS'] = 'Adresse';
	$text['BOOKMARK_LIST'] = 'Liste de liens';
	$text['BOOKMARKS'] = 'Liens';
	$text['BOOKMARKS_ADMINISTRATION'] = 'Administration des liens';
	$text['MODIFICATION_OF'] = 'Modification de';
	$text['PRIVATE'] = 'Privé';
}
_lang($text);


function bookmark_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return bookmark_modify($args);
	print('<h1 class="title bookmark">'._html_safe(BOOKMARKS_ADMINISTRATION)
			.'</h1>'."\n");
	$bookmarks = _sql_array('SELECT bookmark_id AS id, title AS name'
			.', enabled, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id');
	if(!is_array($bookmarks))
		return _error('Could not list bookmarks');
	$count = count($bookmarks);
	for($i = 0; $i < $count; $i++)
	{
		$bookmarks[$i]['module'] = 'bookmark';
		$bookmarks[$i]['action'] = 'display';
		$bookmarks[$i]['icon'] = 'icons/16x16/bookmark.png';
		$bookmarks[$i]['thumbnail'] = 'icons/48x48/bookmark.png';
		$bookmarks[$i]['enabled'] = $bookmarks[$i]['enabled']
			== SQL_TRUE ? 'enabled' : 'disabled';
		$bookmarks[$i]['enabled'] = '<img src="icons/16x16/'
				.$bookmarks[$i]['enabled'].'.png" alt="'
				.$bookmarks[$i]['enabled'].'" title="'
				.($bookmarks[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$bookmarks[$i]['url'] = '<a href="'
			._html_safe($bookmarks[$i]['url']).'">'
			._html_safe($bookmarks[$i]['url'])."</a>";
		$bookmarks[$i]['apply_module'] = 'bookmark';
		$bookmarks[$i]['apply_id'] = $bookmarks[$i]['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_BOOKMARK, 'class' => 'new',
			'link' => _module_link('bookmark', 'new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => PRIVATE, 'class' => 'disabled',
			'action' => 'disable', 'confirm' => 'private');
	$toolbar[] = array('title' => PUBLIC, 'class' => 'enabled',
			'action' => 'enable', 'confirm' => 'publish');
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $bookmarks,
				'class' => array('enabled' => PUBLIC,
					'url' => ADDRESS),
				'view' => 'details', 'toolbar' => $toolbar,
				'module' => 'bookmark', 'action' => 'admin'));
}


function bookmark_default($args)
{
	if(isset($args['id']))
		return bookmark_display($args);
	if(isset($args['user_id']))
		return bookmark_list($args);
	include('./modules/bookmark/default.tpl');
}


function bookmark_delete($args)
{
	global $user_id;

	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT content_id FROM daportal_content'
			." WHERE user_id='$user_id'"
			." AND content_id='".$args['id']."'")) == FALSE)
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	_sql_query('DELETE FROM daportal_bookmark'
			." WHERE bookmark_id='".$args['id']."'");
	_content_delete($id);
}


function bookmark_disable($args)
{
	global $user_id;

	if(!$user_id || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(_sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['id']."'") != $user_id)
		return _error('Could not update bookmark');
	_content_disable($args['id']);
}


function bookmark_display($args)
{
	global $user_id;

	$id = $args['id'];
	$bookmark = _sql_array('SELECT bookmark_id AS id, user_id, enabled'
			.', title, content, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id'
			." AND (user_id='$user_id' OR enabled='1')"
			." AND bookmark_id='$id'");
	if(!is_array($bookmark) || count($bookmark) != 1)
		return _error('Unable to display bookmark');
	$bookmark = $bookmark[0];
	$title = $bookmark['title'];
	include('./modules/bookmark/display.tpl');
}


function bookmark_enable($args)
{
	global $user_id;

	if(!$user_id || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(_sql_single('SELECT user_id FROM daportal_content'
			." WHERE content_id='".$args['id']."'") != $user_id)
		return _error('Could not update bookmark');
	_content_enable($args['id']);
}


function bookmark_insert($args)
{
	global $user_id;

	if(!$user_id || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	$enabled = $args['enabled'] == 'on' ? 1 : 0;
	if(($id = _content_insert($args['title'], $args['content'], $enabled))
			== FALSE)
		return _error('Unable to insert bookmark content');
	if(!_sql_query('INSERT INTO daportal_bookmark (bookmark_id, url)'
			.' VALUES ('."'$id', '".$args['url']."')"))
		return _error('Unable to insert bookmark', 1);
	bookmark_display(array('id' => $id));
}


function bookmark_list($args)
{
	global $user_id;

	if(!isset($args['user_id']))
		$args['user_id'] = $user_id;
	if(!$args['user_id'])
		return _error(PERMISSION_DENIED);
	print('<h1 class="title bookmark">'._html_safe(BOOKMARK_LIST)
		."</h1>\n");
	$enabled = ($args['user_id'] == $user_id) ? '' : " AND enabled='1'";
	$sql = 'SELECT bookmark_id AS id, title AS name, enabled, url'
		.' FROM daportal_bookmark, daportal_content WHERE'
		.' daportal_bookmark.bookmark_id=daportal_content.content_id'
		." AND user_id='".$args['user_id']."'".$enabled;
	$bookmarks = _sql_array($sql);
	if(!is_array($bookmarks))
		return _error('Could not list bookmarks');
	$count = count($bookmarks);
	for($i = 0; $i < $count; $i++)
	{
		$bookmarks[$i]['module'] = 'bookmark';
		$bookmarks[$i]['action'] = 'display';
		$bookmarks[$i]['icon'] = 'icons/16x16/bookmark.png';
		$bookmarks[$i]['thumbnail'] = 'icons/48x48/bookmark.png';
		$bookmarks[$i]['enabled'] = $bookmarks[$i]['enabled']
			== SQL_TRUE ? 'enabled' : 'disabled';
		$bookmarks[$i]['enabled'] = '<img src="icons/16x16/'
				.$bookmarks[$i]['enabled'].'.png" alt="'
				.$bookmarks[$i]['enabled'].'" title="'
				.($bookmarks[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$bookmarks[$i]['url'] = '<a href="'
			._html_safe($bookmarks[$i]['url']).'">'
			._html_safe($bookmarks[$i]['url'])."</a>";
		$bookmarks[$i]['apply_module'] = 'bookmark';
		$bookmarks[$i]['apply_id'] = $bookmarks[$i]['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_BOOKMARK, 'class' => 'new',
			'link' => _module_link('bookmark', 'new'));
	if($user_id == $args['user_id'])
	{
		$toolbar[] = array();
		$toolbar[] = array('title' => PRIVATE, 'class' => 'disabled',
				'action' => 'disable', 'confirm' => 'private');
		$toolbar[] = array('title' => PUBLIC, 'class' => 'enabled',
				'action' => 'enable', 'confirm' => 'publish');
		$toolbar[] = array('title' => DELETE, 'class' => 'delete',
				'action' => 'delete', 'confirm' => 'delete');
	}
	$explorer = array('entries' => $bookmarks,
			'class' => array('enabled' => PUBLIC,
				'url' => ADDRESS), 'view' => 'details',
			'module' => 'bookmark', 'action' => 'list');
	if($user_id)
		$explorer['toolbar'] = $toolbar;
	_module('explorer', 'browse_trusted', $explorer);
}


function bookmark_modify($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	if(!is_numeric($id))
		return _error(INVALID_ARGUMENT);
	$bookmark = _sql_array('SELECT bookmark_id AS id, title, content, enabled, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id'
			." AND user_id='$user_id' AND bookmark_id='$id'");
	if(!is_array($bookmark) || count($bookmark) != 1)
		return _error('Unable to modify bookmark');
	$bookmark = $bookmark[0];
	$title = MODIFICATION_OF.' '.$bookmark['title'];
	include('./modules/bookmark/update.tpl');
}


function bookmark_new($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	$title = NEW_BOOKMARK;
	$bookmark = array();
	$bookmark['url'] = isset($args['url'])
		? stripslashes($args['url']) : '';
	$bookmark['title'] = isset($args['title'])
		? stripslashes($args['title']) : '';
	$bookmark['content'] = '';
	$bookmark['enabled'] = SQL_FALSE;
	include('./modules/bookmark/update.tpl');
}


function bookmark_update($args)
{
	global $user_id;

	if(!$user_id || $_SERVER['REQUEST_METHOD'] != 'POST')
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_user_update($args['id'], $args['title'], $args['content'])
			|| !_sql_query('UPDATE daportal_bookmark'
				." SET url='".$args['url']."'"
				." WHERE bookmark_id='".$args['id']."'"))
		return _error('Could not update bookmark');
	if($args['enabled'] == 'on')
		_content_enable($args['id']);
	else
		_content_disable($args['id']);
	return bookmark_display(array('id' => $args['id']));
}

?>
