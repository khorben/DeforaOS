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


//admin is trusted
function admin_list()
{
	$upp = 10; //users per page

	$offset = is_numeric($_GET['offset']) ? $_GET['offset'] * $upp : 0;
	print("\t\t<h3>Listing users");
	if(($res = sql_query('select count(*) from daportal_users;')) == FALSE)
	{
		print("</h3>\n\t\t<p>No users found.</p>\n");
		return 0;
	}
	$count = $res[0]['count'];
	if(($res = sql_query('select userid, username from daportal_users order by username asc limit '.$upp.' offset '.$offset.';')) == FALSE)
	{
		print("</h3>\n\t\t<p>No users found.</p>\n");
		return 0;
	}
	print(' '.($offset + 1).' to '.($offset + $upp)."</h3>\n");
	print("\t\t<form method=\"post\" action=\"index.php\">
<div class=\"headline\">
\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t<input type=\"hidden\" name=\"action\" value=\"moderate\"/>
\t<input type=\"submit\" name=\"type\" value=\"Delete\"/>
</div>
<table>
\t<tr><th></th><th>Username</th></tr>\n");
	$chk = 0;
	while(sizeof($res) >= 1)
	{
		$username = $res[0]['username'];
		print("\t\t<tr><td><input type=\"checkbox\" name=\"id[".($chk++)."]\" value=\"".$res[0]['userid']."\"/></td><td><a href=\"index.php?module=user&amp;action=admin&amp;username=$username\">$username</a></td></tr>\n");
		array_shift($res);
	}
	print("</table>
\t\t</form>\n");
	print("\t\t<p>\n");
	if($_GET['offset'] == 0)
		print('1');
	else
		print('<a href="index.php?module=user&amp;action=admin">1</a>');
	for($i = 1; $i < $count / $upp; $i++)
	{
		if($i == $_GET['offset'])
			print(' | '.($i+1));
		else
			print(" | <a href=\"index.php?module=user&amp;action=admin&amp;offset=$i\">".($i+1).'</a>');
	}
	print("</p>\n");
	return 0;
}


//admin is trusted
function admin_user()
{
	$username = $_GET["username"];
	if(!ereg("^([a-z]){1,9}$", $username)
			|| ($res = sql_query("select userid, email, homepage from daportal_users where username='$username';")) == FALSE)
	{
		print("\t\t<p>User unknown.</p>\n");
		return 0;
	}
	print("\t\t<div>
\t\t<b>Username:</b> $username<br/>
\t\t<b>User id:</b> ".$res[0]['userid']."<br/>
\t\t<b>E-mail:</b> ".$res[0]['email']."<br/>
\t\t<b>Homepage:</b> ".$res[0]['homepage']."<br/>
\t</div>\n");
	return 0;
}


function user_admin()
{
	global $administrator;

	print("\t\t<h1>Users administration</h1>\n");
	if($administrator != 1)
	{
		print("\t\t<p>Access denied.</p>\n");
		return 0;
	}
	if($_GET["username"] != "")
		return admin_user();
	return admin_list();
}


function user_default()
{
	global $userid, $username, $administrator, $moderator;

	print("\t\t<h1><img src=\"modules/user/icon.png\" alt=\"user\"/> ");
	if($userid == 0)
	{
		print("User login</h1>
\t\t<form method=\"post\" action=\"index.php\">
\t\t\t<div>
\t\t\t\tUsername:<br/><input type=\"text\" name=\"username\" size=\"8\"/><br/>
\t\t\t\tPassword:<br/><input type=\"password\" name=\"password\" size=\"8\"/><br/>
\t\t\t\t<input type=\"submit\" value=\"Login\"/>
\t\t\t\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t\t\t\t<input type=\"hidden\" name=\"action\" value=\"login\"/>
\t\t\t\t<input type=\"hidden\" name=\"query\" value=\"".$_SERVER['QUERY_STRING']."\"/>
\t\t\t</div>
\t\t</form>
\t\t<h2>Registration</h2>
\t\t<form method=\"post\" action=\"index.php\">
\t\t\t<div>
\t\t\t\tUsername:<br/><input type=\"text\" name=\"username\" size=\"8\"/><br/>
\t\t\t\tPassword:<br/><input type=\"password\" name=\"password\" size=\"8\"/><br/>
\t\t\t\tRepeat password:<br/><input type=\"password\" name=\"password2\" size=\"8\"/><br/>
\t\t\t\t<input type=\"submit\" value=\"Register\"/>
\t\t\t\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t\t\t\t<input type=\"hidden\" name=\"action\" value=\"register\"/>
\t\t\t</div>
\t\t</form>
\t\t</div>\n");
		return 0;
	}
	if(($user = $_GET["username"]) != "")
	{
		if(($res = sql_query("select email, homepage, moderator from daportal_users where username='$user';")) == FALSE)
		{
			print("User information</h1>\n
\t\t<p>Unknown user.</p>\n");
			return 0;
		}
		print("User information for $user</h1>\n");
		if($res[0]["moderator"] == "t")
			print("\t\t<p>$user is a moderator.</p>\n");
		if($res[0]["email"] != "")
			print("\t\t<div><b>E-mail address</b>: <a href=\"mailto:".$res[0]["email"]."\">".$res[0]["email"]."</a></div>\n");
		if($res[0]["homepage"] != "")
			print("\t\t<div><b>Homepage:</b> <a href=\"".$res[0]["homepage"]."\">".$res[0]["homepage"]."</a></div>\n");
		return 0;
	}
	print("Homepage</h1>
\t\t<p>Welcome to your homepage, $username.</p>
\t\t<div>\n");
	if($administrator == 1)
		print("\t\t\tYou are an <a href=\"index.php?module=admin\">administrator</a>.<br/>\n");
	if($moderator == 1)
		print("\t\t\tYou are a moderator.<br/>\n");
	print("\t\t\t<br/>\n");
	if(($res = sql_query("select email, homepage from daportal_users where userid='$userid';")) == FALSE || sizeof($res) != 1)
	{
		print("\t\t\t<b>Warning:</b> other informations could not be fetched.<br/>
\t\t</div>\n");
	}
	$email = $res[0]['email'];
	$homepage = $res[0]['homepage'];
	print("\t\t\t<b>E-mail address:</b> <a href=\"mailto:$email\">$email</a><br/>
\t\t\t<b>Homepage:</b> <a href=\"$homepage\">$homepage</a><br/>
\t\t</div>\n");
	print("\t\t<h3>Sessions</h3>\n");
	if(($res = sql_query("select sessionid, ip, expires from daportal_sessions where userid='$userid';")) == FALSE)
		print("\t\t<b>Warning:</b> could not fetch sessions.<br/>\n");
	else
	{
		print("\t\t<form action=\"index.php\" method=\"post\">
<div class=\"headline\">
\t<input type=\"submit\" name=\"type\" value=\"Delete\"/>
\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t<input type=\"hidden\" name=\"action\" value=\"sessions\"/>
</div>
<table cellspacing=\"0\">
\t<tr>
\t\t<th></th><th>Session ID</th><th>IP</th><th>Expiration</th>
\t</tr>\n");
		while(sizeof($res) >= 1)
		{
			global $sessionid;
			static $chk = 0;
			$boldi = '';
			$boldo = '';
			if($sessionid == $res[0]["sessionid"])
			{
				$boldi = '<b>';
				$boldo = '</b>';
			}
			print("\t<tr>
\t\t<td><input type=\"checkbox\" name=\"id[".($chk++)."]\" value=\"".$res[0]["sessionid"]."\"/></td><td>");
			print($boldi.$res[0]["sessionid"].$boldo);
			print("</td><td>$boldi".$res[0]["ip"]."$boldo</td><td>$boldi".$res[0]["expires"]."$boldo</td>
\t</tr>\n");
			array_shift($res);
		}
		print("</table>
\t\t</form>\n");
	}
	print("\t\t<form method=\"post\" action=\"index.php\">
\t\t\t<div>
\t\t\t\t<input type=\"submit\" value=\"Logout\"/>
\t\t\t\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t\t\t\t<input type=\"hidden\" name=\"action\" value=\"logout\"/>
\t\t\t</div>
\t\t</form>\n");
	return 0;
}


function user_dump()
{
	global $administrator;

	if($administrator != 1)
		return 0;
	if(($res = sql_query("select userid, username, password, email, homepage from daportal_users;")) == NULL)
		return 0;
	while(sizeof($res) >= 1)
	{
		print("insert into daportal_users (userid, username, password, email, homepage) values ('".$res[0]["userid"]."', '".$res[0]["username"]."', '".$res[0]["password"]."', '".$res[0]["email"]."', '".$res[0]["homepage"]."');\n");
		array_shift($res);
	}
	return 0;
}


function user_install()
{
	//FIXME
	return 1;
}


function user_login()
{
	$username = $_POST["username"];
	if(!eregi("^[a-z]+$", $username))
		exit(1);
	$password = md5($_POST["password"]);
	if(($res = sql_query("select userid, username from daportal_users where username='".$_POST["username"]."' and password='".$password."';")) == FALSE)
	{
		print("\t\t<h1>Authentication failed</h1>
\t\t<p>Please check your username and password information and try again.</p>\n");
		return 0;
	}
	if(sizeof($res) != 1)
	{
		print("\t\t<h1>System error</h1>
\t\t<p>Please contact the webmaster and report this error.</p>\n");
		return 1;
	}
	$userid = $res[0]["userid"];
	$username = $res[0]["username"];
	global $sessionid;
	$date = date("Y-m-d", time() + 31536000);
	$query = "insert into daportal_sessions (sessionid, userid, ip, expires) values ('$sessionid', '$userid', '".$_SERVER['REMOTE_ADDR']."', '$date');\n";
	sql_query($query);
	$query = strlen($_POST['query']) > 0 ? '?'.$_POST['query'] : '';
	header('Location: index.php'.$query);
	return 0;
}


function user_logout()
{
	global $userid, $sessionid;

	if($_SERVER["REQUEST_METHOD"] != "POST")
		return 1;
	sql_query("delete from daportal_sessions where userid='$userid' and sessionid='$sessionid' and ip='".$_SERVER['REMOTE_ADDR']."';");
	header("Location: index.php");
	return 0;
}


function user_moderate()
{
	global $administrator;

	if($administrator != 1)
		return 1;
	$id = $_POST['id'];
	while(sizeof($id) >= 1)
	{
		$userid = array_shift($id);
		sql_query("delete from daportal_users where userid='$userid';");
	}
	header('Location: index.php?module=user&action=admin');
}


function user_register()
{
	global $sessionid;

	if($_SERVER["REQUEST_METHOD"] != "POST")
		return 1;
	$username = $_POST["username"];
	if(!ereg("^[a-z]{1,9}$", $username))
	{
		print("\t\t<h1>Registration error</h1>
\t\t<p>Invalid username.</p>\n");
		return 0;
	}
	if(sql_query("select username from daportal_users where username='$username';") != FALSE)
	{
		print("\t\t<h1>Registration error</h1>
\t\t<p>Username no longer available.</p>\n");
		return 0;
	}
	$password = $_POST["password"];
	if($password == "")
	{
		print("\t\t<h1>Registration error</h1>
\t\t<p>Empty passwords are forbidden.</p>\n");
		return 0;
	}
	if($password != $_POST["password2"])
	{
		print("\t\t<h1>Registration error</h1>
\t\t<p>Passwords do not match</p>\n");
		return 0;
	}
	$password = md5($password);
	sql_query("insert into daportal_users (username, password) values ('$username', '$password');");
	return user_login();
}


function user_sessions()
{
	global $userid;

	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return 1;
	$id = $_POST['id'];
	while(sizeof($id) >= 1)
	{
		$sessionid = array_shift($id);
		sql_query("delete from daportal_sessions where userid='$userid' and sessionid='$sessionid';");
	}
	header('Location: index.php?module=user');
}


function user_uninstall()
{
	//FIXME
	return 1;
}

?>
