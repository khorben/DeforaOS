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


function _user_name($id)
{
	static $cache = array();

	if(array_key_exists($id, $cache))
		return $cache[$id];
	if(($username = _sql_single('SELECT username FROM daportal_user'
			." WHERE user_id='".addslashes($id)."'"))
			== FALSE)
		return FALSE;
	$cache[$id] = $username;
	return $username;
}

?>
