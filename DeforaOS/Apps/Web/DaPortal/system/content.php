<?php
//system/content.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _content_insert($title, $content, $enabled = 0)
{
	global $module_id, $user_id;

	if(!_sql_query('INSERT INTO daportal_content (module_id, user_id'
			.', title, content, enabled)'
			." VALUES ('$module_id', '$user_id', '$title'"
			.", '$content', '$enabled');"))
		return FALSE;
	return _sql_id('daportal_content', 'content_id');
}


function _content_readable($id)
{
	global $user_id;

	require_once('system/user.php');
	if(_user_admin($user_id) == TRUE)
		return TRUE;
	return _sql_single('SELECT enabled FROM daportal_content'
			." WHERE content_id='$id';") == 't';
}


function _content_select($id, $enabled = '')
{
	if(!is_numeric($id))
		return FALSE;
	$and = is_bool($enabled) ? " AND enabled='".$enabled.'"' : '';
	if(($content = _sql_array('SELECT content_id AS id, timestamp, user_id'
			.', title, content, enabled'
			.' FROM daportal_content'
			." WHERE content_id='".$id."'".$and.';')) == FALSE)
		return FALSE;
	return $content[0];
}


function _content_update($id, $title, $content)
{
	return _sql_query('UPDATE daportal_content SET'
			." title='$title', content='$content'"
			." WHERE content_id='$id';");
}

?>
