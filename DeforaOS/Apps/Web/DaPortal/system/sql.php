<?php
//Copyright 2004 Pierre Pronchery
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
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
if(eregi("sql.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../index.php");
	exit(1);
}


function sql_connect($dbhost, $dbport, $dbname, $dbuser, $dbpassword)
{
	global $connection;

	$str = "";
	if($dbhost != "")
		$str.="host=".$dbhost;
	if($dbport != "")
		$str.=" port=".$dbport;
	if($dbname != "")
		$str.=" dbname='".$dbname."'";
	if($dbuser != "")
		$str.=" user='".$dbuser."'";
	if($dbpassword != "")
		$str.=" password='".$dbpassword."'";
	if(!$connection = @pg_connect($str))
		return -1;
	return 0;
}


function sql_query($query)
{
	global $connection;

	if(version_compare(phpversion(), "4.2.0", "<"))
	{
		if(($res = @pg_exec($query)) == FALSE)
			return FALSE;
	}
	else
	{
		if(($res = @pg_query($query)) == FALSE)
			return FALSE;
	}
	if(version_compare(phpversion(), "4.3.0", "<"))
	{
		while($row = pg_fetch_array($res))
			$array[] = $row;
		return $array;
	}
	return pg_fetch_all($res);
}


//tables
function sql_table_create($table, $query)
{
	sql_query("create table ".$table." ".$query.";");
}

function sql_table_drop($table)
{
	sql_query("drop table ".$table.";");
}


//sequences
function sql_sequence_create($sequence)
{
	sql_query("create sequence ".$sequence.";");
}

function sql_sequence_drop($sequence)
{
	sql_query("drop sequence ".$sequence.";");
}


?>
