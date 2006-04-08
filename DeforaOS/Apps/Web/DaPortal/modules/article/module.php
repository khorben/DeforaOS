<?php //modules/articles/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['ARTICLE_BY'] = 'Article by ';
$text['ARTICLE_ON'] = 'on';
$text['ARTICLE_PREVIEW'] = 'Article preview';
$text['ARTICLE_SUBMISSION'] = 'Article submission';
$text['ARTICLES'] = 'Articles';
$text['ARTICLES_ADMINISTRATION'] = 'Articles administration';
$text['MODIFICATION_OF_ARTICLE'] = 'Modification of article';
global $lang;
if($lang == 'fr')
{
	$text['ARTICLE_BY'] = 'Article par ';
	$text['ARTICLE_ON'] = 'le';
	$text['ARTICLE_PREVIEW'] = "Aperçu de l'article";
	$text['ARTICLES_ADMINISTRATION'] = 'Administration des articles';
}
_lang($text);


function _article_insert($article)
{
	global $user_id;

	if(!$user_id)
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	return _content_insert($article['title'], $article['content']);
}


function article_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/article/icon.png" alt=""/> '
		.ARTICLES_ADMINISTRATION.'</h1>'."\n");
	switch($args['sort'])
	{
		case 'username':
			$order = 'username';
			break;
		case 'enabled':
			$order = 'enabled';
			break;
		case 'name':
			$order = 'title';
			break;
		default:
		case 'date':
			$order = 'timestamp';
			break;
	}
	$articles = _sql_array('SELECT content_id AS id, timestamp'
		.', daportal_content.enabled, title, content'
		.', daportal_content.user_id, username'
		.' FROM daportal_content, daportal_user'
		.', daportal_module'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		." AND daportal_module.name='article'"
		.' AND daportal_module.module_id'
		.'=daportal_content.module_id'
		.' ORDER BY '.$order.' DESC;');
	if(!is_array($articles))
		return _error('Unable to list articles');
	for($i = 0, $cnt = count($articles); $i < $cnt; $i++)
	{
		$articles[$i]['module'] = 'article';
		$articles[$i]['apply_module'] = 'article';
		$articles[$i]['action'] = 'modify';
		$articles[$i]['apply_id'] = $articles[$i]['id'];
		$articles[$i]['icon'] = 'modules/article/icon.png';
		$articles[$i]['thumbnail'] = 'modules/article/icon.png';
		$articles[$i]['name'] = $articles[$i]['title'];
		$articles[$i]['username'] = '<a href="index.php?module=user&id='
				.$articles[$i]['user_id'].'">'
				._html_safe_link($articles[$i]['username'])
				.'</a>';
		$articles[$i]['enabled'] = $articles[$i]['enabled'] == 't' ?
			'enabled' : 'disabled';
		$articles[$i]['enabled'] = '<img src="icons/16x16/'
				.$articles[$i]['enabled'].'.png" alt="'
				.$articles[$i]['enabled'].'" title="'
				.($articles[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$articles[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
				strtotime(substr($articles[$i]['timestamp'],
						0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('icon' => 'modules/article/icon.png',
			'title' => 'Submit article',
			'link' => 'index.php?module=article&action=submit');
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable');
	_module('explorer', 'browse_trusted', array(
				'class' => array('username' => AUTHOR,
					'enabled' => ENABLED,
					'date' => DATE),
				'module' => 'article',
				'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'view' => 'details',
				'toolbar' => $toolbar,
				'entries' => $articles));
}


function article_default($args)
{
	if(isset($args['id']))
		return article_display(array('id' => $args['id']));
	return article_list($args);
}


function article_disable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	_content_disable('id');
}


function article_display($args)
{
	require_once('system/content.php');
	if(($article = _content_select($args['id'], 1)) == FALSE)
		return _error(INVALID_ARGUMENT);
	if(($article['username'] = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$article['user_id']."';"))
			== FALSE)
		return _error('Invalid user');
	$long = 1;
	$title = $article['title'];
	$article['date'] = strftime(DATE_FORMAT,
			strtotime(substr($article['timestamp'], 0, 19)));
	include('display.tpl');
}


function article_enable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	_content_enable('id');
}


function article_list($args)
{
	$title = ARTICLES;
	$where = '';
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."';")))
	{
		$title = ARTICLE_BY.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1><img src="modules/article/icon.png" alt=""/> '
			._html_safe($title).'</h1>'."\n");
	$articles = _sql_array('SELECT content_id AS id, timestamp, title'
			.', content, daportal_content.enabled'
			.', daportal_content.user_id, username'
			.' FROM daportal_content, daportal_user'
			.', daportal_module'
			.' WHERE daportal_user.user_id=daportal_content.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.name='article'"
			.' AND daportal_module.module_id'
			.'=daportal_content.module_id'
			.$where
			.' ORDER BY title ASC;');
	if(!is_array($articles))
		return _error('Unable to list articles');
	if(!isset($username))
	{
		foreach($articles as $article)
		{
			$article['date'] = strftime(DATE_FORMAT,
					strtotime(substr($article['timestamp'],
							0, 19)));
			include('display.tpl');
		}
		return;
	}
	for($i = 0, $cnt = count($articles); $i < $cnt; $i++)
	{
		$articles[$i]['module'] = 'article';
		$articles[$i]['action'] = 'default';
		$articles[$i]['icon'] = 'modules/article/icon.png';
		$articles[$i]['thumbnail'] = 'modules/article/icon.png';
		$articles[$i]['name'] = $articles[$i]['title'];
		$articles[$i]['date'] = strftime('%d/%m/%y %H:%M',
				strtotime(substr($articles[$i]['timestamp'],
						0, 19)));
	}
	_module('explorer', 'browse', array(
				'class' => array('date' => 'Date'),
				'view' => 'details',
				'entries' => $articles));
}


function article_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($module_id = _module_id('article')))
		return _error('Could not verify module');
	$article = _sql_array('SELECT content_id AS id, title, content'
			.' FROM daportal_content'
			." WHERE module_id='$module_id'"
			." AND content_id='".$args['id']."';");
	if(!is_array($article) || count($article) != 1)
		return _error('Unable to modify article');
	$article = $article[0];
	$title = MODIFICATION_OF_ARTICLE.' "'.$article['title'].'"';
	include('update.tpl');
}


function article_submit($article)
{
	global $user_id, $user_name;

	//FIXME tweakable?
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	if(isset($article['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article['title'] = stripslashes($article['title']);
		$article['user_id'] = $user_id;
		$article['username'] = $user_name;
		$article['date'] = strftime(DATE_FORMAT);
		$article['content'] = stripslashes($article['content']);
		include('display.tpl');
		unset($title);
		return include('update.tpl');
	}
	if(!isset($article['send']))
	{
		$title = ARTICLE_SUBMISSION;
		return include('update.tpl');
	}
	if(!_article_insert($article))
		return _error('Could not insert article');
	return include('posted.tpl');
}


function article_system($args)
{
	global $title;

	$title.=' - Articles';
}


function article_update($article)
{
	global $user_id, $user_name;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($article['preview']))
	{
		$long = 1;
		$title = ARTICLE_PREVIEW;
		$article['id'] = stripslashes($article['id']);
		$article['title'] = stripslashes($article['title']);
		$article['user_id'] = $user_id;
		$article['username'] = $user_name;
		$article['date'] = strftime(DATE_FORMAT);
		$article['content'] = stripslashes($article['content']);
		include('display.tpl');
		unset($title);
		return include('update.tpl');
	}
	require_once('system/content.php');
	if(!_content_update($article['id'], $article['title'],
				$article['content']))
		return _error('Could not update article');
	return article_display(array('id' => $article['id']));
}

?>
