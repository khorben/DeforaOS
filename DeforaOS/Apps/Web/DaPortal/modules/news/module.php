<?php //modules/news/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['MODIFICATION_OF_NEWS'] = 'Modification of news';
$text['NEWS_ADMINISTRATION'] = 'News administration';
$text['NEWS_BY'] = 'News by';
$text['NEWS_ON'] = 'on';
$text['NEWS_PREVIEW'] = 'News preview';
global $lang;
if($lang == 'fr')
{
	$text['MODIFICATION_OF_NEWS'] = 'Modification de la dépêche';
	$text['NEWS_ADMINISTRATION'] = 'Administration des news';
	$text['NEWS_BY'] = 'Actualités par';
	$text['NEWS_ON'] = 'le';
	$text['NEWS_PREVIEW'] = 'Aperçu de la dépêche';
}
_lang($text);


function _news_insert($news)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	return _content_insert($news['title'], $news['content']);
}


function news_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/news/icon.png" alt=""/> '
		.NEWS_ADMINISTRATION.'</h1>'."\n");
	$order = 'DESC';
	$sort = 'timestamp';
	if(isset($args['sort']))
	{
		$order = 'ASC';
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'enabled';	break;
			case 'name':	$sort = 'title';	break;
			default:	$order = 'DESC';	break;
		}
	}
	$res = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled, title, content'
		.', daportal_content.user_id, username'
		.' FROM daportal_content, daportal_user, daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY '.$sort.' '.$order.';');
	if(!is_array($res))
		return _error('Unable to list news');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'news';
		$res[$i]['apply_module'] = 'news';
		$res[$i]['action'] = 'modify';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'modules/news/icon.png';
		$res[$i]['thumbnail'] = 'modules/news/icon.png';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['username'] = '<a href="index.php?module=user&id='
				.$res[$i]['user_id'].'">'
				._html_safe_link($res[$i]['username'])
				.'</a>';
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED)
				.'"/>';
		$res[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
					strtotime(substr($res[$i]['timestamp'],
							0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('icon' => 'modules/news/icon.png',
			'title' => 'Submit news',
			'link' => 'index.php?module=news&action=submit');
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable');
	_module('explorer', 'browse_trusted', array(
				'class' => array('username' => AUTHOR,
					'enabled' => ENABLED, 'date' => DATE),
				'module' => 'news', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details',
				'entries' => $res));
}


function news_default($args)
{
	if(isset($args['id']))
		return news_display(array('id' => $args['id']));
	news_list($args);
}


function news_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	_content_disable($args['id']);
}


function news_display($args)
{
	require_once('./system/content.php');
	if(($news = _content_select($args['id'], 1)) == FALSE)
		return _error('Invalid news');
	if(($news['username'] = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$news['user_id']."';"))
			== FALSE)
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
	_content_enable($args['id']);
}


function _list_user($user_id, $username)
{
	print('<h1><img src="modules/news/icon.png" alt=""/> '
			._html_safe(NEWS_BY.' '.$username).'</h1>'."\n");
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', daportal_content.enabled, daportal_content.user_id'
			.', username'
			.' FROM daportal_content, daportal_user'
			.', daportal_module'
			.' WHERE daportal_user.user_id=daportal_content.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.name='news'"
			.' AND daportal_module.module_id'
			.'=daportal_content.module_id'
			." AND daportal_content.user_id='$user_id'"
			.' ORDER BY timestamp DESC;');
	if(!is_array($res))
		return _error('Unable to list news');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'news';
		$res[$i]['action'] = 'default';
		$res[$i]['icon'] = 'modules/news/icon.png';
		$res[$i]['thumbnail'] = 'modules/news/icon.png';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['date'] = strftime('%d/%m/%y %H:%M', strtotime(substr(
						$res[$i]['timestamp'], 0, 19)));
	}
	_module('explorer', 'browse', array('class' => array('date' => 'Date'),
				'view' => 'details',
				'entries' => $res));
}

function news_list($args)
{
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."';")))
		return _list_user($args['user_id'], $username);
	print('<h1><img src="modules/news/icon.png" alt=""/> '._html_safe(NEWS)
			.'</h1>'."\n");
	$sql = ' FROM daportal_module, daportal_content, daportal_user'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='news'"
		.' AND daportal_module.module_id'
		.'=daportal_content.module_id';
	$npp = 10;
	$page = isset($args['page']) ? $args['page'] : 1;
	if(($cnt = _sql_single('SELECT COUNT(*)'.$sql.';')) == 0)
		$cnt = 1;
	$pages = ceil($cnt / $npp);
	$page = min($page, $pages);
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id, username'.$sql
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page-1) * $npp, $npp)).';');
	if(!is_array($res))
		return _error('Unable to list news');
	$long = 0;
	foreach($res as $news)
	{
		$news['date'] = strftime(DATE_FORMAT, strtotime(substr(
						$news['timestamp'], 0, 19)));
		include('./modules/news/news_display.tpl');
	}
	_html_paging('index.php?module=news&amp;action=list&amp;', $page,
			$pages);
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
			.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."';");
	if(!is_array($news) || count($news) != 1)
		return _error('Unable to modify news');
	$news = $news[0];
	print('<h1><img src="modules/news/icon.png" alt=""/> '
			.MODIFICATION_OF_NEWS.' "'.$news['title']."\"</h1>\n");
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


function news_submit($news)
{
	global $user_id, $user_name;

	//FIXME tweakable?
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	if(isset($news['preview']))
	{
		$long = 1;
		$title = NEWS_PREVIEW;
		$news['title'] = stripslashes($news['title']);
		$news['user_id'] = $user_id;
		$news['username'] = $user_name;
		$news['date'] = strftime(DATE_FORMAT);
		$news['content'] = stripslashes($news['content']);
		include('./modules/news/news_display.tpl');
		unset($title);
		return include('./modules/news/news_update.tpl');
	}
	if(!isset($news['send']))
	{
		$title = 'News submission';
		return include('./modules/news/news_update.tpl');
	}
	if(!_news_insert($news))
		return _error('Could not insert news');
	include('./modules/news/news_posted.tpl');
	//send mail
	$admins = _sql_array('SELECT username, email FROM daportal_user'
			." WHERE enabled='1' AND admin='1';");
	if(!is_array($admins))
		return _error('Could not list moderators', 0);
	$to = '';
	$comma = '';
	foreach($admins as $a)
	{
		$to.=$comma.$a['username'].' <'.$a['email'].'>';
		$comma = ', ';
	}
	$headers = 'From: DaPortal <www-data@defora.org>'; //FIXME
	$news['title'] = stripslashes($news['title']);
	$news['content'] = stripslashes($news['content']);
	if(!mail($to, '[DaPortal News submission] '.$news['title'],
				$news['content'], $headers))
		_error('Could not send mail to: '.$to, 0);
}


function news_system($args)
{
	global $title;

	$title.=' - News';
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
