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


function search_admin()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	print("\t\t<h1>Search module administration</h1>
\t\t<div>There is nothing to setup here yet.</div>\n");
	return 0;
}


function search_default()
{
	if($_GET["q"] == "")
	{
		print("\t\t<h1>Search</h1>
\t\t<form method=\"get\" action=\"index.php\">
\t\t\t<input type=\"hidden\" name=\"module\" value=\"search\"/>
\t\t\t<input type=\"text\" size=\"30\" name=\"q\"/>
\t\t\t<input type=\"submit\" value=\"Search\"/>
\t\t</form>\n");
		return 0;
	}
	print("\t\t<h1>Search results</h1>
\t\t<form method=\"get\" action=\"index.php\">
\t\t\t<input type=\"hidden\" name=\"module\" value=\"search\"/>
\t\t\t<input type=\"text\" size=\"30\" name=\"q\" value=\"".htmlentities($_GET["q"])."\"/>
\t\t\t<input type=\"submit\" value=\"Search\"/>
\t\t</form>\n");
	if(($res = sql_query("select moduleid, contentid, title from daportal_contents where content like '%".$_GET["q"]."%';")) == FALSE)
	{
		print("\t\t<div>No matches, sorry.</div>\n");
		return 0;
	}
	$i = 1;
	while(sizeof($res) >= 1)
	{
		$module = module_name($res[0]["moduleid"], 1);
		$contentid = $res[0]["contentid"];
		$title = $res[0]["title"];
		print("\t\t<div style=\"margin-bottom: 10px\">
\t\t\t<div>$i. <a href=\"index.php?module=$module&id=$contentid\">$title</a></div>
\t\t\t<div>Module: <a href=\"index.php?module=$module\">$module</a></div>
\t\t\t<div><i>".$_SERVER["SERVER_NAME"].$_SERVER["PHP_SELF"]."?module=$module&id=$contentid</i></div>
\t\t</div>\n");
		$i++;
		array_shift($res);
	}
	return 0;
}


function search_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function search_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


switch($action)
{
	case "admin":
		return search_admin();
	case "install":
		return search_install();
	case "uninstall":
		return search_uninstall();
	default:
		return search_default();
}


?>
