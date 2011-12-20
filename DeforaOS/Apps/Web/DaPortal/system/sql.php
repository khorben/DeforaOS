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


function _sql_array($query)
{
	return _sql_query($query);
}


function _sql_date($timestamp = FALSE)
{
	if($timestamp == FALSE)
		return strftime(DATE_FORMAT);
	return strftime(DATE_FORMAT, strtotime(substr($timestamp, 0, 19)));
}


function _sql_enum($table, $field)
{
	global $engine;

	$db = $engine->getDatabase();
	return $db->enum($engine, $table, $field);
}


function _sql_id($table, $field)
{
	global $engine;

	$db = $engine->getDatabase();
	return $db->lastId($engine, $table, $field);
}


function _sql_offset($offset, $limit)
{
	global $engine;

	$db = $engine->getDatabase();
	return $db->offset($limit, $offset);
}


function _sql_query($query)
{
	global $engine, $db;

	$query = stripslashes(str_replace("\'", "''", $query));
	return $db->query($engine, $query);
}


function _sql_single($query)
{
	if(($res = _sql_query($query)) === FALSE || count($res) != 1)
		return FALSE;
	return $res[0][0];
}

?>
