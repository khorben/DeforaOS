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
if(eregi("module.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../../index.php");
	exit(1);
}


function admin_admin()
{
	global $administrator;

	print("\t\t<h1>Administration</h1>\n");
	if($administrator != 1)
	{
		print("\t\t<div>You have to be an administrator to access this page.</div>\n");
		return 0;
	}
	print("\t\t<div>
\t\t\t<b>Enabled modules:</b>");
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
	print("\n\t\t</div>
\t\t<div>
\t\t\t<b>Disabled modules:</b>");
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
\t\t</div>");
	return 0;
}


function admin_default()
{
	global $administrator;

	print("\t\t<h1>Administration</h1>\n");
	if($administrator != 1)
	{
		print("\t\t<div>You have to be an administrator to access this page.</div>\n");
		return 0;
	}
	print("\t\t<h2>Modules administration</h2>
\t\t<div class=\"headline\">\n");
	if(($res = sql_query("select modulename from daportal_modules where enable='1' order by modulename asc;")) != FALSE)
	{
		while(sizeof($res) >= 1)
		{
			$modulename = $res[0]["modulename"];
			print("\t\t\t<div style=\"display: inline\">
\t\t\t\t<a href=\"index.php?module=".$modulename."&amp;action=admin\">\n");
			if(file_exists("modules/".$modulename."/icon.png"))
				print("\t\t\t\t\t<div style=\"display: table-row\"><img src=\"modules/".$modulename."/icon.png\" alt=\"".$modulename."\"/></div>\n");
			print("\t\t\t\t\t<div style=\"display: table-row\">$modulename</div>
\t\t\t\t</a>
\t\t\t</div>\n");
			array_shift($res);
		}
	}
	print("\t\t</div>\n");
	return 0;
}


function admin_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function admin_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


switch($action)
{
	case "admin":
		return admin_admin();
	case "install":
		return admin_install();
	case "uninstall":
		return admin_uninstall();
	default:
		return admin_default();
}


?>
