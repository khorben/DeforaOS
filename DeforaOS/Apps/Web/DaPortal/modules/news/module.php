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
$text['MODIFICATION_OF_NEWS'] = 'Modification of news';
$text['NEWS_ADMINISTRATION'] = 'News administration';
$text['NEWS_ON'] = 'on';
$text['NEWS_PREVIEW'] = 'News preview';
$text['SUBMIT_NEWS'] = 'Submit news';
global $lang;
if($lang == 'fr')
{
	$text['MODIFICATION_OF_NEWS'] = 'Modification de la dépêche';
	$text['NEWS_ADMINISTRATION'] = 'Administration des dépêches';
	$text['NEWS_ON'] = 'le';
	$text['NEWS_PREVIEW'] = 'Aperçu de la dépêche';
}
_lang($text);


function _news_insert($news)
{
	global $user_id;

	if(!$user_id)
		return FALSE;
	require_once('./system/content.php');
	return _content_insert($news['title'], $news['content']);
}


function news_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title news">'._html_safe(NEWS_ADMINISTRATION)."</h1>\n");
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
		$res[$i]['action'] = 'modify';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'icons/16x16/news.png';
		$res[$i]['thumbnail'] = 'icons/48x48/news.png';
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


function news_default($args)
{
	if(isset($args['id']))
		return news_display(array('id' => $args['id']));
	news_list($args);
}


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


function news_display($args)
{
	print('<h1 class="title news">'._html_safe(NEWS)."</h1>\n");
	require_once('./system/content.php');
	if(($news = _content_select($args['id'], 1)) == FALSE)
		return _error('Invalid news');
	if(($news['username'] = _sql_single('SELECT username FROM daportal_user'
			." WHERE user_id='".$news['user_id']."'")) == FALSE)
		return _error('Invalid user');
	$long = 1;
	$title = $news['title'];
	$news['date'] = strftime(DATE_FORMAT,
			strtotime(substr($news['timestamp'], 0, 19)));
	include('./modules/news/news_display.tpl');
	if(_module_id('comment'))
		_module('comment', 'childs', array('id' => $news['id']));
}


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


function news_headline($args)
{
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
		$news[$i]['action'] = 'display';
		$news[$i]['icon'] = 'icons/16x16/news.png';
		$news[$i]['thumbnail'] = 'icons/48x48/news.png';
		$news[$i]['name'] = $news[$i]['title'];
	}
	_module('explorer', 'browse', array('toolbar' => 0, 'view' => 'details',
				'header' => 0, 'entries' => $news));
}


function _list_user($user_id, $username)
{
	print('<h1 class="title news">'._html_safe(NEWS._BY_.' '.$username)
			."</h1>\n");
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
		.', daportal_content.enabled, daportal_content.user_id'
		.', username, name AS module'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		." AND daportal_content.user_id='$user_id'"
		.' ORDER BY timestamp DESC');
	if(!is_array($res))
		return _error('Unable to list news');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['action'] = 'default';
		$res[$i]['icon'] = 'icons/16x16/news.png';
		$res[$i]['thumbnail'] = 'icons/48x48/news.png';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['date'] = strftime('%d/%m/%y %H:%M', strtotime(substr(
						$res[$i]['timestamp'], 0, 19)));
	}
	_module('explorer', 'browse', array('class' => array('date' => 'Date'),
				'view' => 'details', 'entries' => $res));
}

function news_list($args)
{
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."'")) != FALSE)
		return _list_user($args['user_id'], $username);
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
	foreach($res as $news)
	{
		$news['date'] = strftime(DATE_FORMAT, strtotime(substr(
						$news['timestamp'], 0, 19)));
		include('./modules/news/news_display.tpl');
	}
	_html_paging(_html_link('news', 'list').'&amp;', $page, $pages);
}


function news_modify($args)
{
	global $user_id, $user_name;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($module_id = _module_id('news')))
		return _error('Could not verify module');
	$news = _sql_array('SELECT content_id AS id, title, content, enabled'
		.' FROM daportal_content WHERE module_id='."'$module_id'"
		." AND content_id='".$args['id']."'");
	if(!is_array($news) || count($news) != 1)
		return _error('Unable to modify news');
	$news = $news[0];
	print('<h1 class="title news">'._html_safe(MODIFICATION_OF_NEWS.' "'
				.$news['title'])."\"</h1>\n");
	$long = 1;
	$title = NEWS_PREVIEW;
	$news['id'] = stripslashes($news['id']);
	$news['title'] = stripslashes($news['title']);
	$news['user_id'] = $user_id; //FIXME keep original user
	$news['username'] = $user_name;
	$news['date'] = strftime(DATE_FORMAT);
	$news['content'] = stripslashes($news['content']);
	include('./modules/news/news_display.tpl');
	unset($title);
	include('./modules/news/news_update.tpl');
}


function news_rss($args)
{
	global $title;

	if(($module_id = _module_id('news')) == FALSE)
		return;
	require_once('./system/html.php');
	$link = 'http://'.$_SERVER['HTTP_HOST'].$_SERVER['SCRIPT_NAME'];
	$content = ''; //FIXME
	include('./modules/news/rss_channel_top.tpl');
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', username'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND module_id='$module_id'"
			." AND daportal_content.enabled='".SQL_TRUE."'"
			.' ORDER BY timestamp DESC '._sql_offset(0, 10));
	if(is_array($res))
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$news = $res[$i];
			$news['date'] = date('D, j M Y H:i:s O', strtotime(
						substr($news['timestamp'], 0,
						       19)));
			$news['link'] = 'http://'.$_SERVER['HTTP_HOST']
				.$_SERVER['SCRIPT_NAME'].'?module=news&id='
				.$news['id'];
			$news['content'] = _html_pre($news['content']);
			include('./modules/news/rss_item.tpl');
		}
	include('./modules/news/rss_channel_bottom.tpl');
}


function news_submit($news)
{
	global $user_id, $user_name;

	//FIXME make it an option
	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$news['user_id'] = $user_id;
	$news['username'] = $user_name;
	if(isset($news['preview']))
	{
		$long = 1;
		$title = NEWS_PREVIEW;
		$news['title'] = stripslashes($news['title']);
		$news['content'] = stripslashes($news['content']);
		$news['date'] = strftime(DATE_FORMAT);
		include('./modules/news/news_display.tpl');
		unset($title);
		return include('./modules/news/news_update.tpl');
	}
	if(!isset($news['send']))
	{
		$title = 'News submission';
		return include('./modules/news/news_update.tpl');
	}
	if(!($news['id'] = _news_insert($news)))
		return _error('Could not insert news');
	include('./modules/news/news_posted.tpl');
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
		.'https://'.$_SERVER['HTTP_HOST'].$_SERVER['SCRIPT_NAME']
		.'?module=news&action=modify&id='.$news['id']."\n"
		."News preview:\n\n"
		."News by ".$news['username']." on ".$news['date']."\n"
		.stripslashes($news['content']);
	require_once('./system/mail.php');
	_mail('Administration Team', $to, '[News submission] '.$news['title'],
			$news['content']);
}


function news_system($args)
{
	global $html, $title;

	$title.=' - News';
	if(isset($args['action']) && $args['action'] == 'rss')
	{
		$html = 0;
		header('Content-Type: text/xml');
	}
}


function news_update($news)
{
	global $user_id, $user_name;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($news['preview']))
	{
		$long = 1;
		$title = NEWS_PREVIEW;
		$news['id'] = stripslashes($news['id']);
		$news['title'] = stripslashes($news['title']);
		$news['user_id'] = $user_id;
		$news['username'] = $user_name;
		$news['date'] = strftime(DATE_FORMAT);
		$news['content'] = stripslashes($news['content']);
		include('./modules/news/news_display.tpl');
		unset($title);
		return include('./modules/news/news_update.tpl');
	}
	require_once('./system/content.php');
	if(!_content_update($news['id'], $news['title'], $news['content']))
		return _error('Could not update news');
	news_display(array('id' => $news['id']));
}

?>
