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


function dump_admin()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	print("\t\t<h1>Database dump</h1>
\t\t<p>
\t\t\tYou can request a <a href=\"index.php?module=dump\">database dump</a> (later as different formats: CSV, PostgreSQL, etc).
\t\t</p>\n");
	return 0;
}


function dump_default()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	print("\t\t<h1>Database dump</h1>\n");
	if(($modules = sql_query("select modulename from daportal_modules;")) == NULL)
	{
		print("\t\t<div>Error while listing modules.</div>\n");
		return 1;
	}
	print("\t\t<pre>\n");
	while(sizeof($modules) >= 1)
	{
		if($modules[0]["modulename"] != "dump")
		{
			print(";dump for module ".$modules[0]["modulename"]."\n");
			module_include($modules[0]["modulename"], "dump");
			print("\n");
		}
		array_shift($modules);
	}
	print("\t\t</pre>\n");
	return 0;
}

function dump_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	//FIXME maybe set a layout for action dump?
	return 0;
}


function dump_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


switch($action)
{
	case "admin":
		return dump_admin();
	case "dump":
		return 0;
	case "install":
		return dump_install();
	case "uninstall":
		return dump_uninstall();
	default:
		return dump_default();
}


?>
