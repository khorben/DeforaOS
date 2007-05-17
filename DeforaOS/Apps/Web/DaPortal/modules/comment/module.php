<?php //modules/comment/module.php
//TODO:
//- enable/disable anonymous comments
//- enable/disable comments per content
//- max nested comments
//- fix comments recursion issues



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['COMMENT_ADMINISTRATION'] = 'Comments administration';
$text['COMMENT_BY'] = 'Comment by';
$text['COMMENT_LIST'] = 'Comment list';
$text['COMMENT_ON'] = 'on';
$text['COMMENT_PREVIEW'] = 'Comment preview';
$text['COMMENT_S'] = 'comment(s)';
$text['COMMENTS_BY'] = 'Comments by';
$text['NEW_COMMENT'] = 'New comment';
global $lang;
if($lang == 'de')
{
	$text['COMMENT_S'] = 'Kommentar';
	$text['NEW_COMMENT'] = 'Neu Kommentar';
}
else if($lang == 'fr')
{
	$text['COMMENT_ADMINISTRATION'] = 'Administration des commentaires';
	$text['COMMENT_BY'] = 'Commentaire de';
	$text['COMMENT_ON'] = 'le';
	$text['COMMENT_S'] = 'commentaire(s)';
	$text['NEW_COMMENT'] = 'Nouveau commentaire';
}
_lang($text);


function comment_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	print('<h1 class="title comment">'.COMMENT_ADMINISTRATION.'</h1>'."\n");
	if(($configs = _config_list('comment')))
	{
		print('<h2 class="title settings">Settings</h2>'."\n");
		$module = 'comment';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title comment">'.COMMENT_LIST.'</h2>'."\n");
	$comments = _sql_array('SELECT daportal_comment.comment_id AS id'
			.', title AS name, daportal_user.user_id AS user_id'
			.', username, daportal_content.enabled AS enabled'
			.', timestamp FROM daportal_comment, daportal_content'
			.', daportal_user WHERE daportal_comment.comment_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' ORDER BY timestamp DESC;');
	if(!is_array($comments))
		return _error('Could not list comments');
	for($i = 0, $cnt = count($comments); $i < $cnt; $i++)
	{
		$comments[$i]['module'] = 'content';
		$comments[$i]['apply_module'] = 'content';
		$comments[$i]['action'] = 'modify';
		$comments[$i]['apply_id'] = $comments[$i]['id'];
		$comments[$i]['icon'] = 'icons/16x16/comment.png';
		$comments[$i]['thumbnail'] = 'icons/48x48/comment.png';
		$comments[$i]['username'] = '<a href="index.php?module=user'
			.'&amp;id='.$comments[$i]['user_id'].'">'
			._html_safe($comments[$i]['username']).'</a>';
		$comments[$i]['enabled'] = $comments[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$comments[$i]['enabled'] = '<img src="icons/16x16/'
				.$comments[$i]['enabled'].'.png" alt="'
				.$comments[$i]['enabled'].'"/>';
		$comments[$i]['date'] = _html_safe(strftime('%d/%m/%Y %H:%M',
				strtotime(substr($comments[$i]['timestamp'], 0,
						19))));
	}
	$toolbar = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	//FIXME does not work
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => DELETE);
	_module('explorer', 'browse_trusted', array('entries' => $comments,
			'class' => array('enabled' => ENABLED,
				'username' => USERNAME, 'date' => DATE),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'comment', 'action' => 'admin'));
}


function comment_childs($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(_user_admin($user_id))
		$where = '';
	else
		$where = " AND daportal_content.enabled='1'"
			." AND child.enabled='1'";
	$parent = $args['id'];
	//FIXME
	$comments = _sql_array('SELECT child.content_id AS id'
			.' FROM daportal_comment, daportal_content'
			.', daportal_content AS child'
			.' WHERE daportal_comment.parent'
			.'=daportal_content.content_id'
			.' AND daportal_comment.comment_id=child.content_id'
			." AND daportal_content.content_id='$parent'".$where
			.' ORDER BY child.timestamp ASC;');
	if(!is_array($comments))
		return _error('Could not display comments');
	foreach($comments as $comment)
		comment_display(array('id' => $comment['id']));
}


function comment_config_update($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^comment_([a-zA-Z_]+)$', $k, $regs))
			_config_set('comment', $regs[1], $args[$k], 0);
	header('Location: index.php?module=comment&action=admin');
	exit(0);
}


function comment_count($args)
{
	$cnt = 0;
	$ids = array();
	$parents = array();

	for($parents[] = $args['id']; $parent = array_shift($parents);)
	{
		$ids[] = $parent;
		$comments = _sql_array('SELECT comment_id FROM daportal_comment'
				.', daportal_content'
				.' WHERE daportal_comment.comment_id'
				.'=daportal_content.content_id'
				." AND enabled='1'"
				." AND parent='$parent';");
		foreach($comments as $c)
		{
			if(in_array($c['comment_id'], $ids))
				continue;
			$cnt++;
			$parents[] = $c['comment_id'];
		}
	}
	return $cnt;
}


function comment_default($args)
{
	if(isset($args['id']))
		return comment_display(array('id' => $args['id']));
	return comment_list($args);
}


function comment_display($args)
{
	global $user_id;

	require_once('./system/user.php');
	$where = " AND daportal_content.enabled='1'";
	if(_user_admin($user_id))
		$where = '';
	$comment = _sql_array('SELECT daportal_comment.comment_id AS id'
			.', daportal_content.enabled AS enabled, timestamp'
			.', title, content, daportal_content.user_id, username'
			.' FROM daportal_comment, daportal_content'
			.', daportal_user'
			.' WHERE daportal_comment.comment_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_comment.comment_id='".$args['id']."'"
			.$where.' ORDER BY timestamp ASC;');
	if(!is_array($comment) || count($comment) != 1)
		return _error('Could not display comment');
	$comment = $comment[0];
	$comment['date'] = strftime(DATE_FORMAT, strtotime(
				substr($comment['timestamp'], 0, 19)));
	include('./modules/comment/display.tpl');
}


function _comment_insert($comment)
{
	require_once('./system/content.php');
	if(!($id = _content_insert($comment['title'], $comment['content'], 1)))
	{
		_error('Could not insert comment');
		return 0;
	}
	if($comment['parent'] == $id)
	{
		_error('Could not insert comment');
		return 0;
	}
	//FIXME endless recursion may happen here
	if(!_sql_query('INSERT INTO daportal_comment (comment_id, parent)'
			.' VALUES ('."'$id', '".$comment['parent']."');"))
	{
		_error('Could not reference comment');
		return 0;
	}
	return $comment['parent'];
}


function comment_list($args)
{
	//FIXME cleanup
	if(isset($args['user_id']) && ($username = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE user_id='".$args['user_id']."';")))
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	else
		return _error('Could not list comments');
	print('<h1 class="title comment">'.COMMENTS_BY.' '._html_safe($username)
		.'</h1>'."\n");
	$comments = _sql_array('SELECT content_id AS id, timestamp'
			.', title AS name, content, daportal_content.user_id'
			.', username, daportal_module.name AS module'
			.' FROM daportal_content, daportal_user'
			.', daportal_module'
			.' WHERE daportal_user.user_id=daportal_content.user_id'
			." AND daportal_content.enabled='1'"
			." AND daportal_module.name='comment'"
			.' AND daportal_module.module_id'
			.'=daportal_content.module_id'
			.$where.' ORDER BY timestamp DESC;');
	if(!is_array($comments))
		return _error('Could not list comments');
	for($i = 0, $cnt = count($comments); $i < $cnt; $i++)
	{
		$comments[$i]['icon'] = 'modules/comment/icon.png';
		$comments[$i]['thumbnail'] = 'modules/comment/icon.png';
		$comments[$i]['action'] = 'display';
		$comments[$i]['date'] = strftime('%d/%m/%y %H:%M',
				strtotime(substr($comments[$i]['timestamp'], 0,
						19)));
	}
	_module('explorer', 'browse', array('view' => 'details',
			'class' => array('date' => DATE),
			'entries' => $comments));
}


function comment_new($args)
{
	global $user_id;

	if($user_id == 0 && _config_get('comment', 'anonymous') != SQL_TRUE)
		return _error(PERMISSION_DENIED);
	print('<h1 class="title comment">'._html_safe(NEW_COMMENT).'</h1>');
	_module('content', 'default', array('id' => $args['parent']));
	$parent = $args['parent'];
	$comment['title'] = 'Re: '._sql_single('SELECT title'
			.' FROM daportal_content'
			." WHERE enabled='1'"
			." AND content_id='$parent';");
	include('./modules/comment/update.tpl');
}


function comment_submit($comment)
{
	global $user_id, $user_name;

	if($user_id == 0 && _config_get('comment', 'anonymous') != SQL_TRUE)
		return _error(PERMISSION_DENIED);
	if(isset($comment['preview']))
	{
		print('<h1 class="title comment">'._html_safe(COMMENT_PREVIEW)
				.'</h1>');
		_module('content', 'default',
				array('id' => $comment['parent']));
		$comment['title'] = stripslashes($comment['title']);
		$comment['user_id'] = $user_id;
		$comment['username'] = stripslashes($user_name);
		$comment['date'] = strftime('%d/%m/%y %H:%M');
		$comment['content'] = stripslashes($comment['content']);
		include('./modules/comment/display.tpl');
		$parent = $comment['parent'];
		return include('./modules/comment/update.tpl');
	}
	if(!_comment_insert($comment))
		return;
	_module('content', 'default', array('id' => $comment['parent']));
	//send mail
	if(($user_email = _sql_single('SELECT email FROM daportal_user'
					." WHERE enabled='1'"
					." AND user_id='$user_id'")) == FALSE)
		$user_email = 'unknown';
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
	$header = 'Reply-To: '.$user_name.' <'.$user_email.'>';
	$comment['title'] = stripslashes($comment['title']);
	$comment['content'] = stripslashes($comment['content']);
	require_once('./system/mail.php');
	_mail('Administration Team', $to, '[Comment submission] '
			.$comment['title'], $comment['content'], $header);
}


function comment_system($args)
{
	global $html;

	if($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['action'])
			&& $_POST['action'] == 'config_update')
		$html = 0;
}

?>
