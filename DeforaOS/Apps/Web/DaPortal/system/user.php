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
	return _sql_single('SELECT admin FROM daportal_user'
			." WHERE user_id='$id';") == 't';
}

?>
