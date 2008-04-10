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



function _content_delete($id)
{
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	return _sql_query('DELETE FROM daportal_content'
			." WHERE content_id='$id'");
}


function _content_disable($id)
{
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	return _sql_query('UPDATE daportal_content SET enabled='."'0'"
			." WHERE content_id='$id'");
}


function _content_display($id)
{
	$sql = 'SELECT name FROM daportal_content, daportal_module'
		.' WHERE daportal_content.module_id=daportal_module.module_id'
		." AND content_id='$id'";
	if(($module = _sql_single($sql)) == FALSE)
		return FALSE;
	_module($module, 'display', array('id' => $id));
	return TRUE;
}


function _content_enable($id)
{
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	return _sql_query('UPDATE daportal_content SET enabled='."'1'"
			." WHERE content_id='$id'");
}


function _content_insert($title, $content, $enabled = 0)
{
	global $module_id, $user_id;

	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	if(!_sql_query('INSERT INTO daportal_content (module_id, user_id'
			.', title, content, enabled)'
			." VALUES ('$module_id', '$user_id', '$title'"
			.", '$content', '$enabled')"))
		return FALSE;
	return _sql_id('daportal_content', 'content_id');
}


function _content_readable($id)
{
	global $user_id;

	require_once('./system/user.php');
	if(_user_admin($user_id) == TRUE)
		return TRUE;
	return _sql_single('SELECT enabled FROM daportal_content'
			." WHERE content_id='$id'") == SQL_TRUE;
}


function _content_select($id, $enabled = '')
{
	global $module_id; /* XXX */

	if(!is_numeric($id))
		return FALSE;
	$and = is_bool($enabled) ? " AND enabled='".$enabled.'"' : '';
	$content = _sql_array('SELECT content_id AS id, timestamp'
			.', daportal_content.user_id AS user_id, username'
			.', title, content, daportal_content.enabled AS enabled'
			.' FROM daportal_content, daportal_user'
			.' WHERE daportal_content.user_id=daportal_user.user_id'
			." AND module_id='$module_id'"
			." AND content_id='$id'".$and);
	if(!is_array($content) || count($content) != 1)
		return FALSE;
	$content = $content[0];
	$content['tag'] = $content['title'];
	_content_select_lang($id, $content['title'], $content['content']);
	return $content;
}


function _content_select_lang($id, &$title, &$content)
{
	global $lang;

	$res = _sql_array('SELECT title, content FROM daportal_content_lang'
			." WHERE content_id='$id' AND lang_id='$lang'");
	if(!is_array($res) || count($res) != 1)
		return;
	$title = $res[0]['title'];
	$content = $res[0]['content'];
}


function _content_user_update($id, $title, $content)
{
	global $user_id;

	require_once('./system/user.php');
	if(_user_admin($user_id))
		return _content_update($id, $title, $content);
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	return _sql_query('UPDATE daportal_content SET'
			." title='$title', content='$content'"
			." WHERE user_id='$user_id' AND content_id='$id'");
}

function _content_update($id, $title, $content = FALSE)
{
	global $module_id;

	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	$sql = 'UPDATE daportal_content SET';
	$sep = '';
	if($title != FALSE)
	{
		$sql.=" title='$title'";
		$sep = ',';
	}
	if($content != FALSE)
		$sql.=$sep." content='$content'";
	$sql.=" WHERE module_id='$module_id' AND content_id='$id'";
	return _sql_query($sql);
}

?>
