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
	global $userid, $username;

	print("\t\t<h1><img src=\"modules/chat/icon.png\" alt=\"chat\"/>Chat</h1>\n");
	$expiration = date("Y-m-d H:i:s", strtotime("-1 day"));
	sql_query("delete from daportal_chat where timestamp < '$expiration';");
	if(($res = sql_query("select author, timestamp, text from daportal_chat order by timestamp desc limit 20;")) != FALSE)
	{
		for($i = sizeof($res) - 1; $i >= 0; $i--)
		{
			print("\t\t<div style=\"font-family: monospace\">");
			$time = explode(' ', $res[$i]['timestamp']);
			$time = explode('+', $time[1]);
			print($time[0].' <font color="');
			if($res[$i]['author'] == $userid)
				print("red\">&lt;</font>$username<font color=\"red\"");
			else if(($res2 = sql_query("select username from daportal_users where userid='".$res[$i]["author"]."';")) != FALSE)
				print("blue\">&lt;</font>".$res2[0]['username']."<font color=\"blue");
			print('">&gt;</font> ');
			print($res[$i]['text']."</div>\n");
		}
	}
	if($userid == 0)
		print("\t\t<p>
\t\t\tYou must be <a href=\"index.php?module=user\">identified</a> to chat.
\t\t</p>\n");
	else
		print("\t\t<form method=\"post\" action=\"index.php\">
\t\t\t<input type=\"text\" size=\"80\" name=\"text\"/>
\t\t\t<input type=\"submit\" value=\"Send\"/>
\t\t\t<input type=\"hidden\" name=\"module\" value=\"chat\"/>
\t\t\t<input type=\"hidden\" name=\"action\" value=\"send\"/>
\t\t</form>\n");
	return 0;
}


function chat_dump()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	if(($res = sql_query('select author, timestamp, text from daportal_chat;')) == NULL)
		return 0;
	while(sizeof($res) >= 1)
	{
		print("insert into daportal_chat (author, timestamp, text) values ('".$res[0]['author']."', '".$res[0]['timestamp']."', '".$res[0]['text']."');\n");
		array_shift($res);
	}
	return 0;
}


function chat_install()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	sql_table_create('daportal_chat', "(
	author integer,
	timestamp timestamp NOT NULL DEFAULT ('now'),
	text varchar(80),
	FOREIGN KEY (author) REFERENCES daportal_users (userid)
)");
	return 0;
}


function chat_send()
{
	global $userid;

	if($userid == 0)
		return 1;
	$author = $userid;
	$text = htmlentities($_POST['text']);
	sql_query("insert into daportal_chat (author, timestamp, text) values ('$author', '".date("Y-m-d H:i:s")."', '$text');");
	header('Location: index.php?module=chat');
	return 0;
}


function chat_uninstall()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	sql_table_drop('daportal_chat');
	return 0;
}


?>
