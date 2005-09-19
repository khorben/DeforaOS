<?php
//system/user.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


//POST	1	is admin
//	0	is not admin
function _user_admin($id)
{
	if(!is_numeric($id))
		return 0;
	/* FIXME cache results? */
	return _sql_single('SELECT admin FROM daportal_user'
			." WHERE user_id='$id' AND enabled='t';") == 't';
}


function _user_id($username)
	/* FIXME check if enabled? */
{
	static $cache = array();

	if(array_key_exists($username, $cache))
		return $cache[$username];
	if(($id = _sql_single('SELECT user_id FROM daportal_user'
			." WHERE username='".addslashes($username)."';"))
			== FALSE)
		return FALSE;
	$cache[$username] = $id;
	return $id;
}

?>
