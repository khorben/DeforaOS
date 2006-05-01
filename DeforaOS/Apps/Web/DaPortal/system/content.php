<?php //system/content.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _content_delete($id)
{
	return _sql_query('DELETE FROM daportal_content'
			." WHERE content_id='$id';");
}


function _content_disable($id)
{
	return _sql_query('UPDATE daportal_content SET enabled='."'f'"
			." WHERE content_id='$id';");
}


function _content_display($id)
{
	if(($module = _sql_single('SELECT name'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND content_id='$id';")) == FALSE)
		return FALSE;
	_module($module, 'display', array('id' => $id));
	return TRUE;
}


function _content_enable($id)
{
	return _sql_query('UPDATE daportal_content SET enabled='."'t'"
			." WHERE content_id='$id';");
}


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

	require_once('./system/user.php');
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

function _content_user_update($id, $title, $content)
{
	global $user_id;

	return _sql_query('UPDATE daportal_content SET'
			." title='$title', content='$content'"
			." WHERE user_id='$user_id' AND content_id='$id';");
}

function _content_update($id, $title, $content)
{
	return _sql_query('UPDATE daportal_content SET'
			." title='$title', content='$content'"
			." WHERE content_id='$id';");
}

?>
