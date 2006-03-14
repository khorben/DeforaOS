<?php
//modules/bookmark/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['ADDRESS'] = 'Address';
$text['BOOKMARK_LIST'] = 'Bookmark list';
$text['BOOKMARKS'] = 'Bookmarks';
$text['NEW_BOOKMARK'] = 'New bookmark';
global $lang;
if($lang == 'fr')
{
	$text['BOOKMARKS'] = 'Liens';
}
_lang($text);


function bookmark_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return bookmark_modify($args);
	print('<h1><img src="modules/admin/icon.png" alt=""/> '
			._html_safe(BOOKMARKS_ADMINISTRATION).'</h1>'."\n");
	bookmark_list(array());
}


function bookmark_default($args)
{
	if(isset($args['id']))
		return bookmark_display($args);
	include('default.tpl');
}


function bookmark_display($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	if(!is_numeric($id))
		return _error(INVALID_ARGUMENT);
	$bookmark = _sql_array('SELECT bookmark_id AS id, title, content, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id'
			." AND enabled='1'"
			." AND user_id='$user_id' AND bookmark_id='$id';");
	if(!is_array($bookmark) || count($bookmark) != 1)
		return _error('Unable to display bookmark');
	$bookmark = $bookmark[0];
	$title = $bookmark['title'];
	include('display.tpl');
}


function bookmark_insert($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], TRUE)) == FALSE)
		return _error('Unable to insert bookmark content');
	if(!_sql_query('INSERT INTO daportal_bookmark (bookmark_id, url)'
			.' VALUES ('."'$id', '".$args['url']."');"))
		return _error('Unable to insert bookmark', 1);
	bookmark_display(array('id' => $id));
}


function bookmark_list($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/bookmark/icon.png" alt=""/> '
			._html_safe(BOOKMARK_LIST).'</h1>'."\n");
	$bookmarks = _sql_array('SELECT bookmark_id AS id, title AS name, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id'
			." AND user_id='$user_id';");
	if(!is_array($bookmarks))
		return _error('Could not list bookmarks');
	$count = count($bookmarks);
	for($i = 0; $i < $count; $i++)
	{
		$bookmarks[$i]['module'] = 'bookmark';
		$bookmarks[$i]['action'] = 'display';
		$bookmarks[$i]['icon'] = 'modules/bookmark/icon.png';
		$bookmarks[$i]['thumbnail'] = 'modules/bookmark/icon.png';
		$bookmarks[$i]['url'] = '<a href="'
			._html_safe_link($bookmarks[$i]['url']).'">'
			._html_safe($bookmarks[$i]['url'])."</a>";
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_BOOKMARK,
			'icon' => 'modules/bookmark/icon.png',
			'link' => 'index.php?module=bookmark&action=new');
	_module('explorer', 'browse_trusted', array(
				'class' => array('url' => ADDRESS),
				'view' => 'details',
				'toolbar' => $toolbar,
				'entries' => $bookmarks));
}


function bookmark_modify($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	$id = $args['id'];
	if(!is_numeric($id))
		return _error(INVALID_ARGUMENT);
	$bookmark = _sql_array('SELECT bookmark_id AS id, title, content, url'
			.' FROM daportal_bookmark, daportal_content'
			.' WHERE daportal_bookmark.bookmark_id'
			.'=daportal_content.content_id'
			." AND enabled='1'"
			." AND user_id='$user_id' AND bookmark_id='$id';");
	if(!is_array($bookmark) || count($bookmark) != 1)
		return _error('Unable to display bookmark');
	$bookmark = $bookmark[0];
	$title = MODIFICATION_OF.' '.$bookmark['title'];
	include('update.tpl');
}


function bookmark_new($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = NEW_BOOKMARK;
	include('update.tpl');
}


function bookmark_update($args)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	if(!_content_user_update($args['id'], $args['title'], $args['content']))
		return _error('Could not update bookmark');
	_sql_query("UPDATE daportal_bookmark SET url='".$args['url']."'"
			." WHERE bookmark_id='".$args['id']."';");
	return bookmark_display(array('id' => $args['id']));
}

?>
