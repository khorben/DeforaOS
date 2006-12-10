<?php //system/user.php



//POST	1	is admin
//	0	is not admin
function _user_admin($id)
{
	static $cache = array();

	if(!is_numeric($id))
		return 0;
	if(array_key_exists($id, $cache))
		return $cache[$id];
	$cache[$id] = _sql_single('SELECT admin FROM daportal_user'
			." WHERE user_id='$id' AND enabled='1'") == SQL_TRUE;
	return $cache[$id];
}


function _user_id($username)
	/* FIXME check if enabled? */
{
	static $cache = array();

	if(array_key_exists($username, $cache))
		return $cache[$username];
	if(($id = _sql_single('SELECT user_id FROM daportal_user'
			." WHERE username='".addslashes($username)."'"))
			== FALSE)
		return FALSE;
	$cache[$username] = $id;
	return $id;
}

?>
