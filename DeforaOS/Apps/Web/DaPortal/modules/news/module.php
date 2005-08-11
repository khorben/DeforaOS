<?php
//modules/news/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['NEWS_ADMINISTRATION'] = 'News administration';
$text['NEWS_BY'] = 'by';
$text['NEWS_ON'] = 'on';
global $lang;
if($lang == 'fr')
{
	$text['NEWS_ADMINISTRATION'] = 'Administration des news';
	$text['NEWS_BY'] = 'par';
	$text['NEWS_ON'] = 'le';
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
	$title = 'News';
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."';")))
	{
		//FIXME list users' news in an explorer instead
		$title.=' by '.$username;
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
	foreach($res as $news)
	{
		$news['date'] = strftime(DATE_FORMAT,
				strtotime(substr($news['timestamp'], 0, 19)));
		include('news_display.tpl');
	}
}


function news_submit($news)
{
	global $user_id, $user_name;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	if(isset($news['preview']))
	{
		$long = 1;
		$title = 'News preview';
		$news['title'] = stripslashes($news['title']);
		$news['user_id'] = $user_id;
		$news['username'] = $user_name;
		$news['date'] = strftime(DATE_FORMAT);
		$news['content'] = stripslashes($news['content']);
		include('news_display.tpl');
		return include('news_update.tpl');
	}
	if(!isset($news['send']))
	{
		print('<h1><img src="modules/news/icon.png" alt=""/> News submission</h1>'."\n");
		return include('news_update.tpl');
	}
	if(!_news_insert($news))
		return _error('Could not insert news');
	return include('news_posted.tpl');
}

?>
