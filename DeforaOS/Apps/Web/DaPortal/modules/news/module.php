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


function display_date($date)
{
	$date = explode("-", $date);
	$date = mktime(0, 0, 0, $date[1], $date[2], $date[0]);
	print(date("l, F jS Y", $date));
}


function display($title, $author, $date, $content)
{
	print("\t\t<div class=\"news\">
\t\t\t<div class=\"news_title\">$title</div>
\t\t\t<div class=\"news_author\">Posted by <a href=\"index.php?module=news&amp;username=$author\">$author</a> on ");
	display_date($date);
	print("</div>
\t\t\t<div class=\"news_content\">$content</div>
\t\t</div>\n");
}

function display_summary($id, $title, $author, $date)
{
	print("\t\t<p class=\"news_summary\">
\t\t\t<div class=\"news_summary_title\"><a href=\"index.php?module=news&amp;id=$id\">$title</a></div>
\t\t\t<div class=\"news_summary_author\">Posted by <a href=\"index.php?module=news&amp;username=$author\">$author</a> on ");
	display_date($date);
	print("</div>
\t\t</p>\n");
}


function news_admin()
{
	global $administrator, $moderator, $moduleid;

	print("\t\t<h1>News administration</h1>\n");
	if($administrator != 1 && $moderator != 1)
	{
		print("\t\t<p>Access denied.</p>\n");
		return 0;
	}
	if(($res = sql_query("select newsid, title, username, date, enable from daportal_contents, daportal_news, daportal_users where moduleid='$moduleid' and contentid=newsid and userid=author;")) == NULL)
	{
		print("\t\t<p>Not any news yet.</p>\n");
		return 0;
	}
	print("\t\t<table>
\t\t\t<tr>
\t\t\t\t<th>Title</th>
\t\t\t\t<th>Author</th>
\t\t\t\t<th>Date</th>
\t\t\t\t<th>Enabled</th>
\t\t\t<tr/>\n");
	while(sizeof($res) >= 1)
	{
		$author = $res[0]["username"];
		$enable = $res[0]["enable"] == "t";
		print("\t\t\t<tr>
\t\t\t\t<td><a href=\"index.php?module=news&amp;id=".$res[0]["newsid"]."\">".$res[0]["title"]."</a></td>
\t\t\t\t<td><a href=\"index.php?module=news&amp;username=$author\">$author</a></td>
\t\t\t\t<td>".$res[0]["date"]."</td>
\t\t\t\t<td><form method=\"post\" action=\"index.php\" style=\"display: inline\">
\t<input type=\"submit\" value=\"".($enable ? "Disable" : "Enable")."\">
\t<input type=\"hidden\" name=\"module\" value=\"news\">
\t<input type=\"hidden\" name=\"action\" value=\"moderate\">
\t<input type=\"hidden\" name=\"newsid\" value=\"".$res[0]["newsid"]."\">
\t<input type=\"hidden\" name=\"enable\" value=\"".($enable ? "0" : "1")."\">
</form></td>
\t\t\t<tr>\n");
		array_shift($res);
	}
	print("\t\t</table>\n");
	return 0;
}


function news_default()
{
	print("\t\t<h1><img src=\"modules/news/icon.png\" alt=\"news\"/>News");
	if(is_numeric($_GET["id"]))
	{
		print("</h1>\n");
		return news_id($_GET["id"]);
	}
	if($_GET["username"] != "" && ereg("^[a-z]{1,9}$", $_GET["username"]))
	{
		print(" by ".$_GET["username"]."</h1>\n");
		return news_username($_GET["username"]);
	}
	print(" summary</h1>
\t\t<p>You can <a href=\"index.php?module=news&amp;action=propose\">propose news</a>.</p>\n");
	return news_summary();
}


function news_summary()
{
	global $moduleid;
	$npp = 10; //news per page

	if(($res = sql_query("select count(*) from daportal_news, daportal_contents where moduleid='$moduleid' and contentid=newsid and enable='1';")) == NULL)
	{
		print("\t\t<p>There aren't any news yet.</p>\n");
		return 0;
	}
	$count = $res[0]["count"];
	$offset = is_numeric($_GET["offset"]) ? $_GET["offset"] * $npp : 0;
	if(($res = sql_query("select title, username, date, content from daportal_news, daportal_contents, daportal_users where moduleid='$moduleid' and enable='1' and contentid=newsid and userid=author order by date desc limit $npp offset $offset;")) != FALSE)
	{
		$first = $offset + 1;
		$last = $first + $npp - 1;
		print("\t\t<h3>Listing news $first to ".min($last, $count)."</h3>\n");
		while(sizeof($res) >= 1)
		{
			display($res[0]["title"],
					$res[0]["username"],
					$res[0]["date"],
					$res[0]["content"]);
			array_shift($res);
		}
		print("\t\t<p>Page: ");
		if($_GET["offset"] == 0)
			print("1");
		else
			print("<a href=\"index.php?module=news&amp;offset=0\">1</a>");
		for($i = 1; $i < $count / $npp; $i++)
		{
			if($i == $_GET["offset"])
				print(" | ".($i+1));
			else
				print(" | <a href=\"index.php?module=news&amp;offset=$i\">".($i+1)."</a>");
		}
		print("</p>\n");
	}
	return 0;
}


function news_dump()
{
	global $administrator, $moduleid;
	require_once("system/contents.php");

	if($administrator != 1)
		return 0;
	contents_dump();
	if(($res = sql_query("select newsid, author, date from daportal_news;")) == NULL)
		return 0;
	while(sizeof($res) >= 1)
	{
		print("insert into daportal_news (newsid, author, date) values ('".$res[0]["newsid"]."', '".$res[0]["author"]."', '".$res[0]["date"]."');\n");
		array_shift($res);
	}
	return 0;
}


function news_id($id)
{
	$query = "select title, username, date, content from daportal_news, daportal_contents, daportal_users where contentid='$id' and newsid=contentid and author=userid";
	if($administrator == 0 && $moderator == 0)
		$query .= " and enable='1'";
	if(($res = sql_query($query)) == FALSE)
	{
		print("\t\t<p>Unknown news.</p>\n");
		return 0;
	}
	display($res[0]["title"], $res[0]["username"],
			$res[0]["date"], $res[0]["content"]);
	return 0;
}


function news_install()
{
	global $administrator, $moduleid;

	if($administrator != 1)
		return 0;
	sql_table_create("daportal_news", "(
	newsid integer,
	author integer,
	date date NOT NULL DEFAULT ('now'),
	FOREIGN KEY (newsid) REFERENCES daportal_contents (contentid),
	FOREIGN KEY (author) REFERENCES daportal_users (userid)
)");
	return 0;
}


//PRE	$number is trusted
function news_last($number)
{
	global $moduleid;

	if(($res = sql_query("select title, username, date, content from daportal_news, daportal_contents, daportal_users where enable='1' and moduleid='$moduleid' and contentid=newsid and userid=author order by date desc limit $number;")) == NULL)
		return 0;
	while(sizeof($res) >= 1)
	{
		display($res[0]["title"], $res[0]["username"],
				$res[0]["date"], $res[0]["content"]);
		array_shift($res);
	}
	return 0;
}


function news_moderate()
{
	global $administrator, $moderator;
	require_once("system/contents.php");

	if($_SERVER["REQUEST_METHOD"] != "POST")
		return 0;
	if($administrator != 1 && $moderator != 1)
		return 0;
	$newsid = $_POST["newsid"];
	$enable = $_POST["enable"];
	if($enable == "1")
		contents_enable($newsid);
	else
		contents_disable($newsid);
	header("Location: index.php?module=news&action=admin");
	return 0;
}


function news_propose()
{
	global $userid;

	print("\t\t<h1>News proposal</h1>\n");
	if($userid == 0)
	{
		print("\t\t<p>
\t\t\tYou must be <a href=\"index.php?module=user\">identified</a> to propose a news.
\t\t</p>\n");
		return 0;
	}

	print("\t\t<form method=\"post\" action=\"index.php\">
\t\t\tTitle: <input type=\"text\" size=\"40\" name=\"title\"/><br/>
\t\t\tContent:<br/>
\t\t\t<textarea cols=\"80\" rows=\"10\" name=\"content\"></textarea><br/>
\t\t\t<input type=\"submit\" value=\"Submit\"/>
\t\t\t<input type=\"hidden\" name=\"module\" value=\"news\"/>
\t\t\t<input type=\"hidden\" name=\"action\" value=\"submit\"/>
\t\t</form>\n");
	return 0;
}


function news_submit()
{
	global $moduleid, $userid;

	if($userid == 0)
		return 1;
	$title = htmlentities($_POST["title"]);
	$content = htmlentities($_POST["content"]);
	$res = sql_query("select nextval('daportal_contents_contentid_seq');");
	$contentid = $res[0]["nextval"];
	sql_query("insert into daportal_contents (contentid, moduleid, title, content) values ('$contentid', '$moduleid', '$title', '$content');");
	sql_query("insert into daportal_news (newsid, author) values ('$contentid', '$userid');");
	header("Location: index.php?module=news&action=thanks");
	exit(0);
}


function news_thanks()
{
	print("\t\t<h1>Thanks!</h1>
\t\t<div>
\t\t\tYour news has been submitted to the webmasters, they will check it as soon as possible. Thanks!
\t\t</div>
\t\t<div>
\t\t\tReturn to the <a href=\"index.php\">main page</a>.
\t\t</div>\n");
	return 0;
}


function news_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	//FIXME remove linked content in daportal_contents
	sql_table_drop("daportal_news");
	return 0;
}


//PRE	$username is a valid username
function news_username($username)
{
	if(($res = sql_query("select newsid, title from daportal_news, daportal_contents, daportal_users where username='$username' and newsid=contentid and author=userid and enable='1';")) == FALSE)
	{
		print("\t\t<p>This user has never posted news</p>\n");
		return 0;
	}
	print("\t\t<p>See <a href=\"index.php?module=user&amp;username=$username\">$username's user informations</a>.</p>\n");
	$i = 1;
	while(sizeof($res) >= 1)
	{
		$id = $res[0]["newsid"];
		$title = $res[0]["title"];
		print("\t\t<p>
\t\t\t$i. <a href=\"index.php?module=news&amp;id=$id\">$title</a><br/>
\t\t\t<i>".$_SERVER["SERVER_NAME"].$_SERVER["PHP_SELF"]."?module=news&amp;id=$id</i>
\t\t</p>\n");
		$i++;
		array_shift($res);
	}
	return 0;
}


switch($action)
{
	case "admin":
		return news_admin();
	case "dump":
		return news_dump();
	case "install":
		return news_install();
	case "last":
		return news_last(1);
	case "last2":
		return news_last(2);
	case "last3":
		return news_last(3);
	case "moderate":
		return news_moderate();
	case "propose":
		return news_propose();
	case "submit":
		return news_submit();
	case "thanks":
		return news_thanks();
	case "uninstall":
		return news_uninstall();
	default:
		return news_default();
}


?>
