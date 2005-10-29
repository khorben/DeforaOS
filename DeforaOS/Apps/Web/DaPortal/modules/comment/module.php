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
	//FIXME
}


function comment_childs($args)
{
	$parent = $args['id'];
	//FIXME
	$comments = _sql_array('SELECT child.content_id AS id'
			.' FROM daportal_comment, daportal_content'
			.', daportal_content AS child'
			.' WHERE daportal_comment.parent'
			.'=daportal_content.content_id'
			.' AND daportal_comment.content_id=child.content_id'
			." AND daportal_content.content_id='$parent';");
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
			." WHERE parent='".$args['id']."';");
}


function comment_default($args)
{
	if(!isset($args['id']))
		return include('default.tpl');
	return comment_display(array('id' => $args['id']));
}


function comment_display($args)
{
	$comment = _sql_array('SELECT daportal_comment.content_id AS id'
			.', timestamp, title'
			.', content, daportal_content.user_id, username'
			.' FROM daportal_comment, daportal_content'
			.', daportal_user'
			.' WHERE daportal_comment.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_comment.content_id='".$args['id']."';");
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
			." WHERE content_id='$parent';");
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
