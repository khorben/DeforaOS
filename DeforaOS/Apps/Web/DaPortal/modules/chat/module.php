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


function chat_admin()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	print("\t\t<h1>Chat module administration</h1>\n");
	print("\t\t<p>
\t\t\tThere isn't anything to setup here yet.
\t\t</div>\n");
	return 0;
}


function chat_default()
{
	print("\t\t<h1>Chat</h1>\n");
	return 0;
}


function chat_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	sql_table_create("daportal_char", "(
	author integer,
	date date NOT NULL DEFAULT ('now'),
	text varchar(80)
)");
	return 0;
}


function chat_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	sql_table_drop("daportal_chat");
	return 0;
}


switch($action)
{
	case "admin":
		return chat_admin();
	case "install":
		return chat_install();
	case "uninstall":
		return chat_uninstall();
	default:
		return chat_default();
}


?>
