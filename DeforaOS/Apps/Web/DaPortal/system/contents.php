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
if(eregi("contents.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../index.php");
	exit(1);
}


function contents_disable($contentid)
{
	global $moduleid;

	sql_query("update daportal_contents set enable='0' where contentid='$contentid' and moduleid='$moduleid';");
}


function contents_dump()
{
	global $moduleid;

	if(($res = sql_query("select contentid, title, content, enable from daportal_contents where moduleid='$moduleid';")) == NULL)
		return 0;
	while(sizeof($res) >= 1)
	{
		print("insert into daportal_contents (contentid, moduleid, title, content, enable) values ('".$res[0]["contentid"]."', '$moduleid', '".$res[0]["content"]."', '".$res[0]["enable"]."');\n");
		array_shift($res);
	}
	return 0;
}


function contents_enable($contentid)
{
	global $moduleid;

	sql_query("update daportal_contents set enable='1' where contentid='$contentid' and moduleid='$moduleid';");
}

?>
