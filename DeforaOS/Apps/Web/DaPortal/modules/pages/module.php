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


function pages_admin()
{
	global $administrator;

	print("\t\t<h1>Pages administration</h1>\n");
	if($administrator != 1)
	{
		print("\t\t<p>Access denied.</p>\n");
		return 0;
	}
	print("\t\t<table>
\t\t\t<tr>
\t\t\t\t<th>Page</th>
\t\t\t\t<th>View</th>
\t\t\t\t<th>Edit</th>
\t\t\t</tr>\n");
	if(($dir = opendir('pages')) != FALSE)
	{
		readdir($dir);
		readdir($dir);
		while($file = readdir($dir))
		{
			if(!eregi('^([a-z]+)\.tpl$', $file, $args))
				continue;
			$page = $args[1];
			print("\t\t\t<tr>
\t\t\t\t<td>$page</td>
\t\t\t\t<td><a href=\"index.php?page=$page\">$page</a></td>
\t\t\t\t<td><a href=\"index.php?module=pages&action=edit&page=$page\">$page</a></td>
\t\t\t</tr>\n");
		}
	}
	print("\t\t</table>\n");
	return 0;
}


function pages_default()
{
	require_once('system/page.php');
	$page = $_GET['page'];
	if($page == '' || !ereg('^[a-z]+$', $page))
		$page = 'index';
	print("\t\t<h1>Page preview: $page</h1>\n");
	return raw_include('pages/'.$page.'.tpl');
}


function pages_dump()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function pages_edit()
{
	require_once('system/raw.php');
	require_once('system/page.php');
	global $administrator;

	if($administrator != 1)
		return 0;
	$page = $_GET['page'];
	if($page == '' || !ereg('^[a-z]+$', $page))
		$page = 'index';
	print("\t\t<h1>Page edition</h1>
\t\t<form method=\"post\" action=\"index.php\">
\t\t<textarea name=\"content\" rows=\"24\" cols=\"80\">");
	raw_include('pages/'.$page.'.tpl');
	print("</textarea>
\t\t\t<br/>
\t\t\t<input type=\"submit\" value=\"Save\"/>
\t\t\t<input type=\"hidden\" name=\"module\" value=\"pages\"/>
\t\t\t<input type=\"hidden\" name=\"action\" value=\"save\"/>
\t\t\t<input type=\"hidden\" name=\"page\" value=\"$page\"/>
\t\t</form>\n");
	if(is_readable('pages/'.$page.'.tpl'))
	{
		print("\t\t<h2>Page preview</h2>
\t\t<div class=\"headline\">\n");
		page_include($page);
		print("\t\t</div>\n");
	}
	return 0;
}


function pages_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function pages_save()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	$page = $_POST['page'];
	if(!ereg('^[a-z]+$', $page))
		return 0;
	if(($fp = fopen("pages/$page.tmp", 'x')) == FALSE)
		return 0;
	
	fwrite($fp, stripslashes($_POST['content']));
	fclose($fp);
	if(file_exists("pages/$page.tpl"))
		@unlink("pages/$page.tpl");
	rename("pages/$page.tmp", "pages/$page.tpl");
	header("Location: index.php?module=pages&action=edit&page=$page");
	return 0;
}


function pages_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


?>
