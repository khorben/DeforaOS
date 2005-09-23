<?php
//modules/news/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['NEWS_ADMINISTRATION'] = 'News administration';
$text['NEWS_BY'] = 'News by ';
$text['NEWS_ON'] = 'on';
$text['NEWS_PREVIEW'] = 'News preview';
global $lang;
if($lang == 'fr')
{
	$text['NEWS_ADMINISTRATION'] = 'Administration des news';
	$text['NEWS_BY'] = 'Actualités par ';
	$text['NEWS_ON'] = 'le';
	$text['NEWS_PREVIEW'] = 'Aperçu de la dépêche';
}
_lang($text);


function _news_insert($news)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	return _content_insert($news['title'], $news['content']);
}


function news_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/news/icon.png" alt=""/> '
			.NEWS_ADMINISTRATION.'</h1>'."\n");
}


function news_default($args)
{
	if(isset($args['id']))
		return news_display(array('id' => $args['id']));
	return news_list($args);
}


function news_display($args)
{
	require_once('system/content.php');
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
	include('news_display.tpl');
}


function news_list($args)
{
	$title = NEWS;
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."';")))
	{
		$title = NEWS_BY.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1><img src="modules/news/icon.png" alt=""/> '.$title.'</h1>'
			."\n");
	$res = _sql_array('SELECT content_id AS id, timestamp'
			.', title, content, daportal_content.user_id, username'
			.' FROM daportal_content, daportal_user'
			.', daportal_module'
			.' WHERE daportal_user.user_id=daportal_content.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.name='news'"
			.' AND daportal_module.module_id'
			.'=daportal_content.module_id'
			.$where
			.' ORDER BY timestamp DESC;');
	if(!is_array($res))
		return _error('Unable to list news');
	if(!isset($username))
	{
		foreach($res as $news)
		{
			$news['date'] = strftime(DATE_FORMAT, strtotime(substr(
						$news['timestamp'], 0, 19)));
			include('news_display.tpl');
		}
		return;
	}
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
	_module('explorer', 'browse', array(
				'class' => array('date' => 'Date'),
				'view' => 'details',
				'entries' => $res));
}


function news_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($module_id = _module_id('news')))
		return _error('Could not verify module');
	$news = _sql_array('SELECT content_id AS id, title, content'
			.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."';");
	if(!is_array($news) || count($news) != 1)
		return _error('Unable to modify news');
	$news = $news[0];
	$title = 'Modification of news "'.$news['title'].'"';
	include('news_update.tpl');
}


function news_submit($news)
{
	global $user_id, $user_name;

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
		include('news_display.tpl');
		unset($title);
		return include('news_update.tpl');
	}
	if(!isset($news['send']))
	{
		$title = 'News submission';
		return include('news_update.tpl');
	}
	if(!_news_insert($news))
		return _error('Could not insert news');
	return include('news_posted.tpl');
}


function news_system($args)
{
	global $title;

	$title.=' - News';
}

?>
