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
$text['ARTICLE'] = 'Article';
$text['ARTICLE_ON'] = 'on';
$text['ARTICLE_PREVIEW'] = 'Article preview';
$text['ARTICLE_SUBMISSION'] = 'Article submission';
$text['ARTICLES'] = 'Articles';
$text['ARTICLES_ADMINISTRATION'] = 'Articles administration';
$text['MODIFICATION_OF_ARTICLE'] = 'Modification of article';
$text['SUBMIT_ARTICLE'] = 'Submit article';
global $lang;
if($lang == 'fr')
{
	$text['ARTICLE_ON'] = 'le';
	$text['ARTICLE_PREVIEW'] = "Aperçu de l'article";
	$text['ARTICLES_ADMINISTRATION'] = 'Administration des articles';
	$text['MODIFICATION_OF_ARTICLE'] = "Modification de l\'article";
}
_lang($text);


function _article_insert($article)
{
	global $user_id;

	if($user_id == 0) //FIXME make it an option
		return FALSE;
	require_once('./system/content.php');
	return _content_insert($article['title'], $article['content']);
}


function article_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title article">'._html_safe(ARTICLES_ADMINISTRATION)
			."</h1>\n");
	$order = 'ASC';
	$sort = 'timestamp';
	if(isset($args['sort']))
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'daportal_content.enabled';
								break;
			case 'name':	$sort = 'title';	break;
			case 'date':
			default:	$order = 'DESC';	break;
		}
	$res = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled AS enabled, title AS name, content'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='article'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY '.$sort.' '.$order);
	if(!is_array($res))
		return _error('Unable to list articles');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'article';
		$res[$i]['apply_module'] = 'article';
		$res[$i]['action'] = 'modify';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'icons/16x16/article.png';
		$res[$i]['thumbnail'] = 'icons/48x48/article.png';
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
	$toolbar[] = array('title' => SUBMIT_ARTICLE, 'class' => 'new',
			'link' => _module_link('article', 'submit'));
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
				'module' => 'article', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details'));
}


function article_default($args)
{
	if(isset($args['id']))
		return article_display(array('id' => $args['id']));
	article_list($args);
}


function article_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_delete($args['id']))
		return _error('Could not delete article');
}


function article_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable article');
}


function article_display($args)
{
	require_once('./system/content.php');
	if(($article = _content_select($args['id'], 1)) == FALSE)
		return _error(INVALID_ARGUMENT);
	$long = 1;
	$title = $article['title'];
	$article['date'] = _sql_date($article['timestamp']);
	include('./modules/article/display.tpl');
}


function article_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable article');
}


function article_list($args)
{
	$title = ARTICLES;
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."'")) != FALSE)
	{
		$title = ARTICLES._BY_.' '.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1 class="title article">'._html_safe($title).'</h1>'."\n");
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
		.', daportal_content.enabled AS enabled'
		.', daportal_content.user_id AS user_id'
		.', username, name AS module'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='article'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' AND daportal_module.module_id'
		.'=daportal_content.module_id'.$where
		.' ORDER BY title ASC');
	if(!is_array($res))
		return _error('Unable to list articles');
	if(!isset($username))
	{
		$long = 0;
		foreach($res as $article)
		{
			$article['date'] = _sql_date($article['timestamp']);
			include('./modules/article/display.tpl');
		}
		return;
	}
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['action'] = 'default';
		$res[$i]['icon'] = 'icons/16x16/article.png';
		$res[$i]['thumbnail'] = 'icons/48x48/article.png';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['date'] = strftime('%d/%m/%y %H:%M', strtotime(substr(
						$res[$i]['timestamp'], 0, 19)));
	}
	$toolbar = array();
	$toolbar[] = array('title' => SUBMIT_ARTICLE, 'class' => 'new',
			'link' => _module_link('article', 'submit'));
	_module('explorer', 'browse', array('class' => array('date' => DATE),
				'view' => 'details', 'entries' => $res,
				'toolbar' => $toolbar));
}


function article_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($module_id = _module_id('article')) == FALSE)
		return _error('Could not verify module');
	$article = _sql_array('SELECT content_id AS id, title, content'
			.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."'");
	if(!is_array($article) || count($article) != 1)
		return _error('Unable to modify article');
	$article = $article[0];
	$title = MODIFICATION_OF_ARTICLE.' "'.$article['title'].'"';
	include('./modules/article/update.tpl');
}


function article_submit($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		return _error($error);
	if(isset($args['send']))
	{
		return include('./modules/article/posted.tpl');
	}
	if(isset($args['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article = array('user_id' => $user_id,
				'username' => $user_name,
				'title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']),
				'date' => strftime(DATE_FORMAT),
				'preview' => 1);
		include('./modules/article/display.tpl');
		unset($title);
	}
	else
		$title = ARTICLE_SUBMISSION;
	include('./modules/article/update.tpl');
}


function article_system($args)
{
	global $title;

	$title.=' - '.ARTICLES;
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'submit':
			return _system_article_submit($args);
		case 'update':
			return _system_article_update($args);
	}
}

function _system_article_submit($args)
{
	global $error, $user_id, $user_name;

	if($user_id == 0) //FIXME make it an option
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(isset($args['preview']) || !isset($args['send']))
		return;
	$article = array('user_id' => $user_id, 'username' => $user_name,
			'title' => $args['title'],
			'content' => $args['content']);
	if(!($article['id'] = _article_insert($article)))
	{
		$error = 'Could not insert article';
		return;
	}
	_submit_send_mail($article);
	header('Location: '._module_link('article', 'submit', FALSE, FALSE,
				'send='));
	exit(0);
}

function _submit_send_mail($article)
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
	$article['title'] = stripslashes($article['title']);
	$article['date'] = strftime(DATE_FORMAT);
	$article['content'] = "News is available for moderation at:\n"
		._module_link('article', 'modify', $article['id'])."\n"
		."Article preview:\n\n"
		."Article by ".$article['username']." on ".$article['date']."\n"
		.stripslashes($article['content']);
	require_once('./system/mail.php');
	_mail('Administration Team', $to, '[Article submission] '
			.$article['title'], $article['content']);
}

function _system_article_update($args)
{
	global $error, $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(isset($args['preview']))
		return;
	if(!is_numeric($args['id']))
	{
		$error = INVALID_ARGUMENT;
		return;
	}
	require_once('./system/content.php');
	if(!_content_update($args['id'], $args['title'], $args['content']))
	{
		$error = 'Could not update article';
		return;
	}
	header('Location: '._module_link('article', FALSE, $args['id'],
				$args['title']));
	exit(0);
}


function article_update($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		return _error($error);
	if(!is_numeric($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($article = _content_select($args['id'])) == FALSE)
		return _error(INVALID_ARGUMENT);
	if(isset($args['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article['title'] = stripslashes($args['title']);
		$article['date'] = _sql_date($article['timestamp']);
		$article['content'] = stripslashes($args['content']);
		include('./modules/article/display.tpl');
		unset($title);
	}
	include('./modules/article/update.tpl');
}

?>
