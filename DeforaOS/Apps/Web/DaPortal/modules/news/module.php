<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['COMMENT_S'] = 'comment(s)';
$text['MODIFICATION_OF_NEWS'] = 'Modification of news';
$text['NEWS_ADMINISTRATION'] = 'News administration';
$text['NEWS'] = 'News';
$text['NEWS_ON'] = 'on';
$text['NEWS_RE'] = 'Re';
$text['NEWS_PREVIEW'] = 'News preview';
$text['NEWS_SUBMISSION'] = 'News submission';
$text['REPLY_TO_NEWS'] = 'Reply to news';
$text['SUBMIT_NEWS'] = 'Submit news';
global $lang;
if($lang == 'fr')
{
	$text['COMMENT_S'] = 'commentaire(s)';
	$text['MODIFICATION_OF_NEWS'] = 'Modification de la dépêche';
	$text['NEWS_ADMINISTRATION'] = 'Administration des dépêches';
	$text['NEWS_ON'] = 'le';
	$text['NEWS_PREVIEW'] = 'Aperçu de la dépêche';
}
_lang($text);


//private
//news_display
function _news_display($id, $title = NEWS)
{
	require_once('./system/content.php');
	if(($news = _content_select($id, 1)) == FALSE)
	{
		_error(INVALID_ARGUMENT);
		return FALSE;
	}
	$long = 1;
	$news['date'] = _sql_date($news['timestamp']);
	include('./modules/news/news_display.tpl');
	return TRUE;
}


//news_insert
function _news_insert($news)
{
	global $user_id;

	if($user_id == 0) //FIXME make it an option
		return FALSE;
	require_once('./system/content.php');
	return _content_insert($news['title'], $news['content']);
}


//public
//news_admin
function news_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	require_once('./system/icon.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title news">'._html_safe(NEWS_ADMINISTRATION)
			."</h1>\n");
	$order = 'DESC';
	$sort = 'timestamp';
	if(isset($args['sort']))
	{
		$order = 'ASC';
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'daportal_content.enabled';
								break;
			case 'name':	$sort = 'title';	break;
			default:	$order = 'DESC';	break;
		}
	}
	$res = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled AS enabled, title AS name, content'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY '.$sort.' '.$order);
	if(!is_array($res))
		return _error('Unable to list news');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'news';
		$res[$i]['apply_module'] = 'news';
		$res[$i]['action'] = 'update';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = _icon('news', 16);
		$res[$i]['thumbnail'] = _icon('news', 48);
		$res[$i]['name'] = _html_safe($res[$i]['name']);
		$res[$i]['username'] = '<a href="'._html_link('user', '',
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
					strtotime(substr($res[$i]['timestamp'],
							0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('title' => SUBMIT_NEWS, 'class' => 'new',
			'link' => _module_link('news', 'submit'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'username' => AUTHOR, 'date' => DATE),
				'module' => 'news', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details'));
}


//news_default
function news_default($args)
{
	if(!isset($args['id']))
		return news_list($args);
	if(_news_display($args['id']) && _module_id('comment'))
		_module('comment', 'childs', array('id' => $args['id']));
}


//news_delete
function news_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_delete($args['id']))
		return _error('Could not delete news');
}


//news_disable
function news_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable news');
}


//news_enable
function news_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable news');
}


//news_headline
function news_headline($args)
{
	require_once('./system/icon.php');
	$page = 1;
	$npp = 10;
	if(isset($args['npp']) && is_numeric($args['npp']))
		$npp = $args['npp'];
	$news = _sql_array('SELECT content_id AS id, title, name AS module'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_module.name='news'"
			." AND daportal_content.enabled='1'"
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page-1) * $npp, $npp)));
	if(!is_array($news))
		return _error('Could not list news');
	for($i = 0, $cnt = count($news); $i < $cnt; $i++)
	{
		$news[$i]['action'] = 'default';
		$news[$i]['icon'] = _icon('news', 16);
		$news[$i]['thumbnail'] = _icon('news', 48);
		$news[$i]['name'] = $news[$i]['title'];
		$news[$i]['tag'] = $news[$i]['title'];
	}
	_module('explorer', 'browse', array('toolbar' => 0, 'view' => 'details',
				'header' => 0, 'entries' => $news));
}


//news_list
function news_list($args)
{
	require_once('./system/user.php');
	if(isset($args['user_id'])
			&& ($username = _user_name($args['user_id'])) != FALSE)
		return _news_list_user($args['user_id'], $username);
	print('<h1 class="title news">'._html_safe(NEWS)."</h1>\n");
	$sql = ' FROM daportal_module, daportal_content, daportal_user'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id=daportal_content.module_id';
	$npp = 10;
	$page = isset($args['page']) ? $args['page'] : 1;
	if(($cnt = _sql_single('SELECT COUNT(*)'.$sql)) == 0)
		$cnt = 1;
	$pages = ceil($cnt / $npp);
	$page = min($page, $pages);
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'.$sql
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page-1) * $npp, $npp)));
	if(!is_array($res))
		return _error('Unable to list news');
	$long = 0;
	require_once('./system/content.php');
	foreach($res as $news)
	{
		$news['tag'] = $news['title'];
		_content_select_lang($news['id'], $news['title'],
				$news['content']);
		$news['date'] = _sql_date($news['timestamp']);
		include('./modules/news/news_display.tpl');
	}
	_html_paging(_html_link('news', 'list', FALSE, FALSE, 'page='), $page,
			$pages);
}

function _news_list_user($user_id, $username)
{
	require_once('./system/icon.php');
	print('<h1 class="title news">'._html_safe(NEWS._BY_.' '.$username)
			."</h1>\n");
	$res = _sql_array('SELECT content_id AS id, timestamp AS date, title'
		.', content, daportal_content.enabled AS enabled'
		.', daportal_content.user_id AS user_id'
		.', username, name AS module'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		." AND daportal_content.user_id='$user_id'"
		.' ORDER BY date DESC');
	if(!is_array($res))
		return _error('Unable to list news');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['action'] = 'default';
		$res[$i]['icon'] = _icon('news', 16);
		$res[$i]['thumbnail'] = _icon('news', 48);
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['date'] = strftime('%d/%m/%Y %H:%M', strtotime(substr(
						$res[$i]['date'], 0, 19)));
	}
	$toolbar = array();
	$toolbar[] = array('title' => SUBMIT_NEWS, 'class' => 'new',
			'link' => _module_link('news', 'submit'));
	_module('explorer', 'browse', array('class' => array('date' => DATE),
				'view' => 'details', 'entries' => $res,
				'toolbar' => $toolbar));
}


//news_reply
function news_reply($args)
{
	global $error;

	if(isset($error) && strlen($error))
		return _error($error);
	if(!_module_id('comment'))
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	$id = $args['id'];
	if(!($title = _sql_single('SELECT title FROM daportal_content'
					." WHERE content_id='$id'")))
		return _error(INVALID_ARGUMENT);
	$title = NEWS_RE.': '.$title;
	$content = '';
	//display news
	_news_display($id, REPLY_TO_NEWS);
	//check if there is a preview
	$parent = isset($args['parent']) ? $args['parent'] : $args['id'];
	if(($preview = isset($args['preview'])))
	{
		$title = isset($args['title']) ? $args['title'] : $title;
		$content = isset($args['content']) ? $args['content']
			: $content;
	}
	//present form
	_module('comment', 'reply', array('module' => 'news', 'id' => $id,
				'parent' => $parent, 'preview' => $preview,
				'title' => $title, 'content' => $content));
}


//news_rss
function news_rss($args)
{
	global $title;

	if(($module_id = _module_id('news')) == FALSE)
		return;
	require_once('./system/html.php');
	$link = _module_link_full('news');
	$atomlink = _module_link_full('news', 'rss');
	$content = $title;
	$res = _sql_array('SELECT content_id AS id, timestamp AS date, title'
			.', content, username AS author, email'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND module_id='$module_id'"
			." AND daportal_content.enabled='1'"
			.' ORDER BY timestamp DESC '._sql_offset(0, 10));
	if(is_array($res))
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$res[$i]['author'] = $res[$i]['email']
				.' ('.$res[$i]['author'].')';
			$res[$i]['date'] = date('D, j M Y H:i:s O', strtotime(
						substr($res[$i]['date'], 0,
						19)));
			$res[$i]['link'] = _html_link_full('news', FALSE,
					$res[$i]['id']);
			$res[$i]['content'] = _html_pre($res[$i]['content']);
		}
	require_once('./system/rss.php');
	_rss($title, $link, $atomlink, $content, $res);
}


//news_submit
function news_submit($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		return _error($error);
	if(isset($args['send']))
	{
		return include('./modules/news/news_posted.tpl');
	}
	$title = NEWS_SUBMISSION;
	if(isset($args['preview']))
	{
		$long = 1;
		$title = NEWS_PREVIEW;
		$news = array('user_id' => $user_id,
				'username' => $user_name,
				'title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']),
				'date' => strftime(DATE_FORMAT),
				'preview' => 1);
		include('./modules/news/news_display.tpl');
		unset($title);
	}
	include('./modules/news/news_update.tpl');
}


//news_system
function news_system($args)
{
	global $html, $title, $error;

	$title.=' - '.NEWS;
	if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		if(isset($args['action']) && $args['action'] == 'rss')
		{
			$html = 0;
			header('Content-Type: text/xml');
		}
		return;
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
		switch($args['action'])
		{
			case 'reply':
				$error = _system_news_reply($args);
				return;
			case 'submit':
				$error = _system_news_submit($args);
				return;
			case 'update':
				$error = _system_news_update($args);
				return;
		}
}

function _system_news_reply($args)
{
	global $error;

	if(!_module_id('comment'))
		return PERMISSION_DENIED;
	if(!isset($args['id']))
		return INVALID_ARGUMENT;
	if(isset($args['preview']))
		return;
	$args['parent'] = isset($args['parent']) ? $args['parent']
		: $args['id'];
	_module('comment', 'insert', $args);
	if(strlen($error) != 0)
		return $error;
	header('Location: '._module_link('news', FALSE, $args['id'],
				FALSE));
	exit(0);
}

function _system_news_submit($args)
{
	global $user_id, $user_name;

	if($user_id == 0) //FIXME make it an option
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	$news = array('user_id' => $user_id, 'username' => $user_name,
			'title' => $args['title'],
			'content' => $args['content']);
	if(!($news['id'] = _news_insert($news)))
		return 'Could not insert news';
	_submit_send_mail($news);
	header('Location: '._module_link('news', 'submit', FALSE, FALSE,
				'send='));
	exit(0);
}

function _submit_send_mail($news)
{
	//send mail
	$admins = _sql_array('SELECT username, email FROM daportal_user'
			." WHERE enabled='1' AND admin='1'");
	if(!is_array($admins))
		return _error('Could not list moderators', 0);
	$to = '';
	$comma = '';
	foreach($admins as $a)
	{
		$to.=$comma.$a['username'].' <'.$a['email'].'>';
		$comma = ', ';
	}
	$news['title'] = stripslashes($news['title']);
	$news['date'] = strftime(DATE_FORMAT);
	$news['content'] = "News is available for moderation at:\n"
		._module_link_full('news', 'update', $news['id'])."\n"
		."News preview:\n\n"
		."News by ".$news['username']." on ".$news['date']."\n"
		.stripslashes($news['content']);
	require_once('./system/mail.php');
	_mail('Administration Team', $to, '[News submission] '.$news['title'],
			wordwrap($news['content'], 72));
}

function _system_news_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	if(!isset($args['id']) || !isset($args['title'])
			|| !isset($args['content']))
		return INVALID_ARGUMENT;
	if(isset($args['preview']))
		return;
	require_once('./system/content.php');
	if(!_content_update($args['id'], $args['title'], $args['content']))
		return 'Could not update news';
	header('Location: '._module_link('news', FALSE, $args['id'],
				$args['title']));
	exit(0);
}


//news_update
function news_update($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		_error($error);
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($news = _content_select($args['id'])) == FALSE)
		return _error(INVALID_ARGUMENT);
	print('<h1 class="title news">'._html_safe(MODIFICATION_OF_NEWS.': '
				.$news['title'])."</h1>\n");
	if(isset($args['preview']))
	{
		$long = 1;
		$news['title'] = stripslashes($args['title']);
		$news['date'] = _sql_date($news['timestamp']);
		$news['content'] = stripslashes($args['content']);
		include('./modules/news/news_display.tpl');
	}
	include('./modules/news/news_update.tpl');
}

?>
