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



if($action == "admin" && $administrator = 1)
{
	print("\t<h1>Users administration</h1>\n");

	if($_GET["username"] != "")
	{
		$username = $_GET["username"];
		if(!ereg("^[a-z]+$", $username)
				|| ($res = sql_query("select userid from daportal_users where username='$username'")) == FALSE)
		{
			print("\t<div>
\t\tInvalid user.<br/>
\t</div>\n");
			return 0;
		}
		print("\t<div>
\t\t<b>Username:</b> $username<br/>
\t\t<b>User id:</b> ".$res[0]["userid"]."<br/>
\t</div>\n");
		return 0;
	}
	print("\t<div>
\t\t<b>Registered users:</b>");
	if(($res = sql_query("select username from daportal_users")) != FALSE)
	{
		while(sizeof($res) >= 1)
		{
			$username = $res[0]["username"];
			print(" <a href=\"index.php?module=user&action=admin&username=$username\">$username</a>");
			array_shift($res);
		}
	}
	else
		print(" none.");
	print("<br/>
\t</div>\n");
	return 0;
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
\t\t<div>
\t\t\tWelcome to your homepage, $username.<br/>
\t\t\t<br/>\n");
	if($administrator == 1)
		print("\t\t\tYou are an <a href=\"index.php?module=admin\">administrator</a>.<br/>\n");
	if($moderator == 1)
		print("\t\t\tYou are a moderator.<br/>\n");
	print("\t\t\t<br/>\n");
	if(($res = sql_query("select email, homepage from daportal_users where userid='$userid';")) == FALSE || sizeof($res) != 1)
	{
		print("\t\t\t<b>Warning:</b> other informations could not be fetched.<br/>
\t</div>\n");
	}
	$email = $res[0]["email"];
	$homepage = $res[0]["homepage"];
	print("\t\t\t<b>E-mail address:</b> $email<br/>
\t\t\t<b>Homepage:</b> $homepage<br/>
\t\t</div>\n");
	print("\t\t<h3>Sessions</h3>\n");
	if(($res = sql_query("select sessionid, ip, expires from daportal_sessions where userid='$userid';")) == FALSE)
		print("\t\t\t<b>Warning:</b> could not fetch sessions.<br/>\n");
	else
	{
		print("\t\t\t<table cellspacing=\"0\">
\t\t\t\t<tr>
\t\t\t\t\t<th>Session ID</th><th>IP</th><th>Expiration</th>
\t\t\t\t</tr>");
		while(sizeof($res) >= 1)
		{
			print("\t\t\t\t<tr>
\t\t\t\t\t<td>".$res[0]["sessionid"]."</td><td>".$res[0]["ip"]."</td><td>".$res[0]["expires"]."</td>
\t\t\t\t</tr>");
			array_shift($res);
		}
		print("\t\t\t</table>\n");
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
	{
		print(";error while dumping users\n");
		return 0;
	}
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
		return 1;
	if(sizeof($res) != 1)
		return 1;
	$userid = $res[0]["userid"];
	$username = $res[0]["username"];
	global $sessionid;
	$date = date("Y-m-d", time() + 31536000);
	$query = "insert into daportal_sessions (sessionid, userid, ip, expires) values ('$sessionid', '$userid', '".$_SERVER['REMOTE_ADDR']."', '$date');\n";
	sql_query($query);
	header("Location: index.php");
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


function user_register()
{
	global $sessionid;

	if($_SERVER["REQUEST_METHOD"] != "POST")
		return 1;
	$username = $_POST["username"];
	if(!ereg("^[a-z]+$", $username))
	{
		print("Invalid username\n");
		return 1;
	}
	if(sql_query("select username from daportal_users where username='$username';") != FALSE)
	{
		print("Username no longer available\n");
		return 1;
	}
	$password = $_POST["password"];
	if($password == "")
	{
		print("Empty passwords are forbidden\n");
		return 1;
	}
	if($password != $_POST["password2"])
	{
		print("Passwords do not match\n");
		return 1;
	}
	$password = md5($password);
	sql_query("insert into daportal_users (username, password) values ('$username', '$password');");
	return user_login();
}


function user_uninstall()
{
	//FIXME
	return 1;
}


switch($action)
{
	case "dump":
		return user_dump();
	case "install":
		return user_install();
	case "login":
		return user_login();
	case "logout":
		return user_logout();
	case "register":
		return user_register();
	case "uninstall":
		return user_uninstall();
	default:
		return user_default();
}


?>
