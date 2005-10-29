<?php
//modules/comment/module.php
//TODO:
//- enable/disable anonymous comments
//- enable/disable comments per content



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['COMMENT_ADMINISTRATION'] = 'Comments administration';
$text['COMMENT_BY'] = 'Comment by';
$text['COMMENT_ON'] = 'on';
$text['COMMENT_PREVIEW'] = 'Comment preview';
$text['COMMENT_S'] = 'comment(s)';
$text['NEW_COMMENT'] = 'New comment';
global $lang;
if($lang == 'fr')
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

	require_once('system/user.php');
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	print('<h1><img src="modules/comment/icon.png" alt=""/> '
		.COMMENT_ADMINISTRATION.'</h1>'."\n");
	$comments = _sql_array('SELECT daportal_comment.content_id AS id'
			.', title AS name, enabled, timestamp'
			.' FROM daportal_comment, daportal_content'
			.' WHERE daportal_comment.content_id'
			.'=daportal_content.content_id'
			.' ORDER BY timestamp DESC;');
	if(!is_array($comments))
		return _error('Could not list comments');
	for($i = 0, $cnt = count($comments); $i < $cnt; $i++)
	{
		$comments[$i]['module'] = 'content';
		$comments[$i]['action'] = 'modify';
		$comments[$i]['apply_id'] = $comment[$i]['id'];
		$comments[$i]['icon'] = 'modules/comment/icon.png';
		$comments[$i]['thumbnail'] = 'modules/comment/icon.png';
		$comments[$i]['enabled'] = $comments[$i]['enabled'] == 't' ?
			'enabled' : 'disabled';
		$comments[$i]['enabled'] = '<img src="icons/16x16/'
				.$comments[$i]['enabled'].'.png" alt="'
				.$comments[$i]['enabled'].'"/>';
		$comments[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
				strtotime(substr($comments[$i]['timestamp'], 0,
						19))));
	}
	_module('explorer', 'browse_trusted', array(
			'class' => array('enabled' => ENABLED, 'date' => DATE),
			'entries' => $comments,
			'view' => 'details'));
}


function comment_childs($args)
{
	global $user_id;

	require_once('system/user.php');
	if(_user_admin($user_id))
		$where = '';
	else
		$where = " AND daportal_content.enabled='t'"
			." AND child.enabled='t'";
	$parent = $args['id'];
	//FIXME
	$comments = _sql_array('SELECT child.content_id AS id'
			.' FROM daportal_comment, daportal_content'
			.', daportal_content AS child'
			.' WHERE daportal_comment.parent'
			.'=daportal_content.content_id'
			.' AND daportal_comment.content_id=child.content_id'
			." AND daportal_content.content_id='$parent'"
			.$where
			.' ORDER BY child.timestamp ASC;');
	if(!is_array($comments))
	{
		_error('Could not display comments');
		return;
	}
	foreach($comments as $comment)
		comment_display(array('id' => $comment['id']));
}


function comment_count($args)
{
	//FIXME does not recurse
	return _sql_single('SELECT COUNT(*) FROM daportal_comment'
			.', daportal_content'
			.' WHERE daportal_comment.content_id'
			.'=daportal_content.content_id'
			." AND enabled='t'"
			." AND parent='".$args['id']."';");
}


function comment_default($args)
{
	if(!isset($args['id']))
		return include('default.tpl');
	return comment_display(array('id' => $args['id']));
}


function comment_display($args)
{
	global $user_id;

	require_once('system/user.php');
	if(_user_admin($user_id))
		$where = '';
	else
		$where = " AND daportal_content.enabled='t'";
	$comment = _sql_array('SELECT daportal_comment.content_id AS id'
			.', daportal_content.enabled, timestamp, title'
			.', content, daportal_content.user_id, username'
			.' FROM daportal_comment, daportal_content'
			.', daportal_user'
			.' WHERE daportal_comment.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_comment.content_id='".$args['id']."'"
			.$where
			.' ORDER BY timestamp ASC;');
	if(!is_array($comment) || count($comment) != 1)
		return _error('Could not display comment');
	$comment = $comment[0];
	$comment['date'] = strftime(DATE_FORMAT, strtotime(
				substr($comment['timestamp'], 0, 19)));
	include('display.tpl');
}


function _comment_insert($comment)
{
	require_once('system/content.php');
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
	if(!_sql_query('INSERT INTO daportal_comment (content_id, parent)'
			.' VALUES ('."'$id', '".$comment['parent']."');"))
	{
		_error('Could not reference comment');
		return 0;
	}
	return $comment['parent'];
}


function comment_new($args)
{
	print('<h1><img src="modules/comment/icon.png" alt=""/> '
			._html_safe(NEW_COMMENT).'</h1>');
	_module('content', 'default', array('id' => $args['parent']));
	$parent = $args['parent'];
	$comment['title'] = 'Re: '._sql_single('SELECT title'
			.' FROM daportal_content'
			." WHERE enabled='t'"
			." AND content_id='$parent';");
	include('update.tpl');
}


function comment_submit($comment)
{
	if(isset($comment['preview']))
	{
		print('<h1><img src="modules/comment/icon.png" alt=""/> '
				._html_safe(COMMENT_PREVIEW).'</h1>');
		_module('content', 'default',
				array('id' => $comment['parent']));
		include('display.tpl');
		$parent = $comment['parent'];
		return include('update.tpl');
	}
	if(($id = _comment_insert($comment)))
		return _module('content', 'default', array('id'
					=> $comment['parent']));
}

?>
