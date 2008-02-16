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
//TODO:
//- enable/disable anonymous comments
//- enable/disable comments per content
//- max nested comments
//- fix comments recursion issues



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['COMMENT_ADMINISTRATION'] = 'Comments administration';
$text['COMMENT_BY'] = 'Comment by';
$text['COMMENT_LIST'] = 'Comment list';
$text['COMMENT_ON'] = 'on';
$text['COMMENT_RE'] = 'Re: ';
$text['SETTINGS'] = 'Settings';
global $lang;
if($lang == 'de')
{
	$text['COMMENT_BY'] = 'Kommentar von';
	$text['COMMENT_ON'] = 'am';
}
else if($lang == 'fr')
{
	$text['COMMENT_BY'] = 'Commentaire de';
	$text['COMMENT_ON'] = 'le';
	$text['SETTINGS'] = 'Paramètres';
}
_lang($text);


//private
//comment_display
function _comment_display($module, $parent, $id)
{
	global $user_id;

	$where = '';
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		$where = " AND enabled='1'";
	$comment = _sql_array('SELECT comment_id AS id, title, user_id'
			.', timestamp AS date, content'
			.' FROM daportal_comment, daportal_content'
			.' WHERE daportal_comment.comment_id'
			.'=daportal_content.content_id'.$where
			." AND comment_id='$id'");
	if(!is_array($comment) || count($comment) != 1)
		return 'Could not display comment';
	$comment = $comment[0];
	$comment['module'] = $module;
	$comment['parent'] = $parent;
	$comment['username'] = _user_name($comment['user_id']);
	$comment['date'] = _sql_date($comment['date']);
	include('./modules/comment/display.tpl');
}


//comment_parents
function _comment_parents($module, $id)
{
	$ids = array();
	$module = '';
	for($child = $id;; $child = $parent)
	{
		$res = _sql_array('SELECT parent.content_id AS id'
				.', parent.module_id AS module_id'
				.' FROM daportal_content parent'
				.', daportal_comment child'
				." WHERE parent.enabled='1'"
				.' AND child.parent=parent.content_id'
				." AND child.comment_id='$child'");
		if(!is_array($res) || count($res) != 1
				|| in_array($res[0]['id'], $ids))
			break;
		$parent = $res[0]['id'];
		$ids[] = $child;
	}
	while($id = array_pop($ids))
		_comment_display($module, $child, $id);
	return TRUE;
}


//public
//comment_admin
function comment_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!$user_id)
		return _error(PERMISSION_DENIED);
	print('<h1 class="title comment">'.COMMENT_ADMINISTRATION."</h1>\n");
	if(($configs = _config_list('comment')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'comment';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title comment">'._html_safe(COMMENT_LIST)."</h2>\n");
	$sql = 'SELECT daportal_comment.comment_id AS id, title AS name'
		.', daportal_user.user_id AS user_id, username'
		.', daportal_content.enabled AS enabled, timestamp'
		.' FROM daportal_comment, daportal_content, daportal_user'
		.' WHERE daportal_comment.comment_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' ORDER BY timestamp DESC';
	$comments = _sql_array($sql);
	if(!is_array($comments))
		return _error('Could not list comments');
	for($i = 0, $cnt = count($comments); $i < $cnt; $i++)
	{
		$comments[$i]['module'] = 'content';
		$comments[$i]['apply_module'] = 'content';
		$comments[$i]['action'] = 'update';
		$comments[$i]['apply_id'] = $comments[$i]['id'];
		$comments[$i]['icon'] = 'icons/16x16/comment.png';
		$comments[$i]['thumbnail'] = 'icons/48x48/comment.png';
		$comments[$i]['username'] = '<a href="'._html_link('user', '',
			$comments[$i]['user_id'], $comments[$i]['username'])
				.'">'._html_safe($comments[$i]['username'])
				.'</a>';
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
	//FIXME still does not work?
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => DELETE);
	_module('explorer', 'browse_trusted', array('entries' => $comments,
			'class' => array('enabled' => ENABLED,
				'username' => USERNAME, 'date' => DATE),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'comment', 'action' => 'admin'));
}


//comment_config_update
function comment_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return comment_admin(array());
}


//comment_count
function comment_count($args)
	//FIXME return a string instead (to keep the translations here)
{
	$cnt = 0;
	$parents = array();
	$ids = array();

	if(!isset($args['id']))
		return 0;
	for($parents[] = $args['id']; $parent = array_shift($parents);)
	{
		$ids[] = $parent;
		$comments = _sql_array('SELECT comment_id AS id'
				.' FROM daportal_comment, daportal_content'
				.' WHERE daportal_comment.comment_id'
				.'=daportal_content.content_id'
				." AND enabled='1' AND parent='$parent'");
		if(!is_array($comments))
			return 0;
		foreach($comments as $c)
		{
			if(in_array($c['id'], $ids))
				continue;
			$cnt++;
			$parents[] = $c['id'];
		}
	}
	return $cnt;
}


//comment_childs
function comment_childs($args)
{
	global $user_id;

	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	$parent = $args['id'];
	$module = _sql_single('SELECT name FROM daportal_content'
			.', daportal_module WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_module.enabled='1'"
			." AND content_id='$parent'");
	$where = '';
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		$where = " AND daportal_content.enabled='1'";
	$ids = array();
	$parents = array();
	for($parents[] = $args['id']; $parent = array_shift($parents);)
	{
		$ids[] = $parent;
		$comments = _sql_array('SELECT comment_id AS id'
				.' FROM daportal_comment, daportal_content'
				.' WHERE daportal_comment.comment_id'
				.'=daportal_content.content_id'
				." AND daportal_comment.parent='$parent'"
				.$where);
		if(!is_array($comments))
			return _error('Could not display comments');
		foreach($comments as $c)
		{
			if(in_array($c['id'], $ids))
				continue;
			_comment_display($module, $args['id'], $c['id']);
			$parents[] = $c['id'];
		}
	}
}


//comment_default
function comment_default($args)
{
	if(isset($args['id']))
	{
		_comment_display(array('id' => $args['id']));
		comment_childs(array('id' => $args['id']));
	}
	_error(INVALID_ARGUMENT);
}


//comment_insert
function comment_insert($comment)
{
	global $user_id, $user_name, $error;

	if($user_id == 0 && _config_get('comment', 'anonymous') != TRUE)
	{
		$error = PERMISSION_DENIED;
		return;
	}
	if(!isset($comment['title']) || !isset($comment['content'])
			|| !isset($comment['parent'])
			|| !_sql_single('SELECT content_id'
				.' FROM daportal_content'." WHERE enabled='1'"
				." AND content_id='".$comment['parent']."'"))
	{
		$error = INVALID_ARGUMENT;
		return;
	}
	require_once('./system/content.php');
	if(!($id = _content_insert($comment['title'], $comment['content'], 1)))
	{
		$error = 'Could not insert comment';
		return;
	}
	if(!_sql_query('INSERT INTO daportal_comment (comment_id, parent)'
				." VALUES ('$id', '".$comment['parent']."')"))
	{
		$error = 'Could not reference comment';
		_content_delete($id);
		return;
	}
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
			.$comment['title'], wordwrap($comment['content'], 72),
			$header);
}


//comment_reply
function comment_reply($args)
{
	global $user_id, $user_name;

	if(!isset($args['module']) || !isset($args['id'])
			|| !isset($args['parent']))
		return _error(INVALID_ARGUMENT);
	//display parents
	$comment = array();
	$comment['module'] = $args['module'];
	$comment['parent'] = $args['parent'];
	if(_comment_parents($comment['module'], $comment['parent']) == FALSE)
		return;
	//populate comment
	$comment['title'] = isset($args['title']) ? stripslashes($args['title'])
		: '';
	$comment['date'] = _sql_date();
	$comment['user_id'] = $user_id;
	$comment['username'] = $user_name;
	$comment['content'] = isset($args['content'])
		? stripslashes($args['content']) : '';
	//check if there is a preview
	if(isset($args['preview']) && $args['preview'] == TRUE)
	{
		if(isset($args['title']))
			$comment['title'] = stripslashes($args['title']);
		if(isset($args['content']))
			$comment['content'] = stripslashes($args['content']);
		$comment['preview'] = 1;
		include('./modules/comment/display.tpl');
		unset($comment['preview']);
	}
	$comment['id'] = $args['id'];
	//generate title if not provided
	if(!isset($args['title']) && ($res = _sql_single('SELECT title'
					.' FROM daportal_content'
					." WHERE content_id='$parent'")))
		$comment['title'] = COMMENT_RE.': '.$res;
	include('./modules/comment/update.tpl');
}


//comment_system
function comment_system($args)
{
	global $user_id, $error;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		exit(0);
	if(!isset($args['action']) || $_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'config_update':
			$error = _system_comment_config_update($args);
			break;
	}
}

function _system_comment_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	$args['comment_anonymous'] = isset($args['comment_anonymous']) ? TRUE
		: FALSE;
	_config_update('comment', $args);
	header('Location: '._module_link('comment', 'admin'));
	exit(0);
}

?>
