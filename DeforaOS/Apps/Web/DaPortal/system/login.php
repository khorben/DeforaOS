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
if(eregi("login.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../index.php");
	exit(1);
}


$userid = 0;
$username = "";
$administrator = 0;
$moderator = 0;

session_set_cookie_params(31536000);
session_start();
if(($sessionid = session_id()) == "")
	return 1;
sql_query("delete from daportal_sessions where expires < 'now()';");
$res = sql_query("select daportal_sessions.userid, username, administrator, moderator from daportal_sessions, daportal_users where sessionid='$sessionid' and ip='".$_SERVER['REMOTE_ADDR']."' and daportal_sessions.userid=daportal_users.userid;");
if(sizeof($res) != 1)
	return 1;
$userid = $res[0]["userid"];
$username = $res[0]["username"];
$administrator = $res[0]["administrator"] == "t";
$moderator = $res[0]["moderator"] == "t";


?>
