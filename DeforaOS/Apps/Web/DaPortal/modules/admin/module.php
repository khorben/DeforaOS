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



global $administrator;
if($administrator != 1)
	return 0;

function admin_install()
{
	return 0;
}


function admin_summary()
{
	print("\t<div>
\t\t<h1>Administration</h1>
\t\tThe administrator's page.<br/>
\t\t<br/>
\t\t<b>Enabled modules:</b>");
	if(($res = sql_query("select modulename from daportal_modules where enable='1'")) != FALSE)
	{
		while(sizeof($res) >= 1)
		{
			print(" <a href=\"index.php?module=".$res[0]["modulename"]."&action=admin\">".$res[0]["modulename"]."</a>");
			array_shift($res);
		}
	}
	else
		print(" none.");
	print("<br/>
\t\t<b>Disabled modules:</b>");
	if(($res = sql_query("select modulename from daportal_modules where enable='0'")) != FALSE)
	{
		while(sizeof($res) >= 1)
		{
			print(" ".$res[0]["modulename"]);
			array_shift($res);
		}
	}
	else
		print(" none.");
	print("<br/>
\t</div>");
	return 0;
}


switch($action)
{
	case "install":
		return admin_install();
	default:
		return admin_summary();
}


?>
