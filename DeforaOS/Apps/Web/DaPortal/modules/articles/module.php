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


function articles_admin()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	print("\t<h1>Articles administration</h1>\n");
	return 0;
}


function articles_default()
{
	if($_GET['id'] != "")
		return articles_view($_GET['id']);
	print("\t<h1>Articles list</h1>\n");
	return 0;
}


function articles_dump()
{
	global $administrator;
	require_once('system/contents.php');

	if($administrator != 1)
		return 0;
	return contents_dump();
}


function articles_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function articles_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	return 0;
}


function articles_view($id)
{
	global $moduleid, $administrator, $moderator;

	$query = "select title, content from daportal_contents where contentid='$id' and moduleid='$moduleid'";
	if($administrator == 0 && $moderator == 0)
		$query .= " and enable='1'";
	if(!is_numeric($id)
			|| ($res = sql_query($query)) == NULL
			|| sizeof($res) != 1)
	{
		print("\t<h1>Article error</h1>
\t<p>
\t\tWrong article.
\t</p>\n");
		return 0;
	}
	print("\t<h1>".$res[0]['title']."</h1>\n");
	print(htmlentities($res[0]['content']));
	return 0;
}


switch($action)
{
	case "admin":
		return articles_admin();
	case "dump":
		return articles_dump();
	case "install":
		return articles_install();
	case "uninstall":
		return articles_uninstall();
	default:
		return articles_default();
}


?>
