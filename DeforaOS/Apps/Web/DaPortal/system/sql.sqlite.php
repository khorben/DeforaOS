<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));

define('SQL_TRUE', 1);
define('SQL_FALSE', 0);


function _query($query)
{
	global $connection;

	$query = stripslashes(str_replace("\'", "''", $query));
	_info($query);
	if(($res = sqlite_query($connection, $query)) === FALSE)
		_error(sqlite_error_string(sqlite_last_error($connection)), 0);
	return $res;
}


function _sql_array($query)
{
	if(($res = _query($query)) === FALSE)
		return FALSE;
	for($array = array(); ($a = sqlite_fetch_array($res)) !== FALSE;
			$array[] = $a);
	return $array;
}


function _sql_enum($table, $field)
{
	if(($res = _query('SELECT name FROM '.$table.'_enum_'.$field))
			=== FALSE)
		return FALSE;
	for($array = array(); ($a = sqlite_fetch_array($res)) !== FALSE;
			$array[] = $a['name']);
	return $array;
}


function _sql_id($table, $field)
{
	global $connection;

	return sqlite_last_insert_rowid($connection);
}


function _sql_offset($offset, $limit)
{
	return 'LIMIT '.$limit.' OFFSET '.$offset;
}


function _sql_single($query)
{
	if(($res = _query($query)) === FALSE)
		return FALSE;
	return sqlite_fetch_single($res);
}


//main
global $dbname;
$error = '';
if(($connection = sqlite_open($dbname, 0666, $error)) === FALSE)
	_error($error, 0);

?>
