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
if(eregi('module.php', $_SERVER['REQUEST_URI']))
{
	header('Location: ../../index.php');
	exit(1);
}


function admin_admin()
{
	global $administrator;

	print("\t\t<h1>Administration</h1>\n");
	if($administrator != 1)
	{
		print("\t\t<p>You have to be an administrator to access this page.</p>\n");
		return 0;
	}
	print("\t\t<h2>Installed modules</h2>\n");
	$installed = array();
	if(($res = sql_query("select moduleid, modulename, enable from daportal_modules order by modulename asc;")) == FALSE)
		print("\t\t<p>Not any module installed.</p>\n");
	else
	{
		print("\t\t<form method=\"post\" action=\"index.php\">
<div class=\"headline\">
\t<input type=\"submit\" name=\"type\" value=\"Enable\"/>
\t<input type=\"submit\" name=\"type\" value=\"Disable\"/>
\t<input type=\"submit\" name=\"type\" value=\"Uninstall\"/>
\t<input type=\"hidden\" name=\"module\" value=\"admin\"/>
\t<input type=\"hidden\" name=\"action\" value=\"modules\"/>
</div>
<table>
\t<tr><th></th><th>Module</th><th>ID</th><th>Enabled</th></tr>\n");
		$i = 0;
		while(sizeof($res) >= 1)
		{
			$module = $res[0]['modulename'];;
			if($res[0]['enable'] == 't')
				$module = "<a href=\"index.php?module=$module&amp;action=admin\">$module<a>";
			print("\t<tr><td><input type=\"checkbox\" name=\"id[$i]\" value=\"".$res[0]['moduleid']."\"/></td><td>$module</td><td>".$res[0]['moduleid']."</td><td>".($res[0]['enable'] == 't' ? 'Yes' : 'No')."</td></tr>\n");
			$installed[] = $res[0]['modulename'];
			array_shift($res);
		}
		print("</table>
\t\t</form>\n");
	}
	print("\t\t<h2>Uninstalled modules</h2>
\t\t<form method=\"post\" action=\"index.php\">
<div class=\"headline\">
\t<input type=\"submit\" name=\"type\" value=\"Install\"/>
\t<input type=\"hidden\" name=\"module\" value=\"admin\"/>
\t<input type=\"hidden\" name=\"action\" value=\"modules\"/>
</div>");
	if(($dir = opendir('modules')) == FALSE)
		print("\t\t<p>Not any module available.</p>\n");
	else
	{
		print("<table>
\t<tr><th></th><th>Module</th></tr>\n");
		readdir($dir);
		readdir($dir);
		while(($dirname = readdir($dir)) != FALSE)
		{
			if(!is_dir($dirname))
				continue;
			if(array_search($dirname, $installed) != FALSE)
				continue;
			print("\t<tr><td><input type=\"checkbox\" name=\"id[$i] value=\"$dirname\"/></td><td>$dirname</td></tr>\n");
		}
		print("</table>\n");
	}
	print("\t\t</form>\n");
	return 0;
}


function admin_default()
{
	global $administrator;

	print("\t\t<h1><img src=\"modules/admin/icon.png\" alt=\"admin\"/>Administration</h1>\n");
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
			$modulename = $res[0]['modulename'];
			print("\t\t\t<div style=\"display: inline\">
\t\t\t\t<a href=\"index.php?module=".$modulename."&amp;action=admin\">\n");
			if(file_exists('modules/'.$modulename.'/icon.png'))
				print("\t\t\t\t\t<div style=\"display: table-row\"><img src=\"modules/".$modulename."/icon.png\" alt=\"".$modulename."\"/></div>\n");
			print("\t\t\t\t\t<div style=\"display: table-row; text-align: center\">$modulename</div>
\t\t\t\t</a>
\t\t\t</div>\n");
			array_shift($res);
		}
	}
	print("\t\t</div>\n");
	return 0;
}


function admin_dump()
{
	global $administrator;

	if($administrator != 1)
		return 0;
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


?>
