<?php //$Id$
//Copyright (c) 2004, 2005, 2006, 2007 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//TODO:
//- test thoroughly on all supported SQL backends



//check url
if(!ereg('/install.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: index.php'));


//variables
$dbtype = FALSE;
$dbhost = FALSE;
$dbport = FALSE;
$dbname = FALSE;
$dbuser = FALSE;
$dbpassword = FALSE;


//installing functions
//install_database
//resets and creates default tables
//PRE
//POST
function install_database($dbtype, $dbhost, $dbport, $dbname, $dbuser,
		$dbpassword)
{
	global $connection;

	//quirks
	switch($dbtype)
	{
		case 'mysql':
			break;
		case 'pgsql':
			break;
		case 'sqlite':
			//XXX security issue but the user is trusted (127.0.0.1)
			if(!file_exists($dbname))
				if(touch($dbname) != TRUE)
					return 1;
			break;
		default: //unknown database
			return 1;
	}
	$connection = FALSE;
	require_once('./system/sql.php');
	if($connection == FALSE)
		return 1;
	if(($sql = file_get_contents('install/'.$dbtype.'.sql')) == FALSE)
		return 1;
	if($dbtype == 'sqlite') //XXX one more quirk
		sqlite_query($connection, $sql); //XXX can't check errors?
	else if(_sql_query($sql) == FALSE)
		return 1;
	return 0;
}

function install_config($dbtype, $dbhost, $dbport, $dbname, $dbuser,
		$dbpassword, $template, $theme)
{
	if(($fp = fopen('config.php', 'w')) == FALSE)
		return 1;
	fwrite($fp, "<?php //config.php


//check url
if(!ereg('/index.php$', \$_SERVER['SCRIPT_NAME']))
	exit(header('Location: index.php'));


\$template = '$template';
\$theme = '$theme';
//\$debug = 1;		//force debugging on
\$dbtype = '$dbtype';	//'mysql', 'pgsql' or 'sqlite'
\$dbhostname = '$dbhost';	//defaults to local connections
\$dbname = '$dbname';
\$dbuser = '$dbuser';
\$dbpassword = '$dbpassword';
\$friendlylinks = 0;	//force \"friendly links\" generation

?>\n");
	fclose($fp);
	return 0;
}

function install_index()
{
	if(($fp = fopen('index.php', 'w')) == FALSE)
		return 1;
	if(fwrite($fp, "<?php require_once('./engine.php'); ?>\n") == FALSE)
	{
		fclose($fp);
		return 1;
	}
	return fclose($fp) == TRUE ? 0 : 1;
}

//
//PRE
//POST	1	error
//	0	success
function install_process($dbtype, $dbhost, $dbport, $dbname, $dbuser,
		$dbpassword, $template, $theme)
{
	if(install_database($dbtype, $dbhost, $dbport, $dbname, $dbuser,
				$dbpassword) != 0
			|| install_config($dbtype, $dbhost, $dbport, $dbname,
				$dbuser, $dbpassword, $template, $theme) != 0
			|| install_index() != 0)
		return 1;
	session_start();
	$_SESSION['user_id'] = 1;
	$_SESSION['user_name'] = 'admin';
	header('Location: index.php?module=user&action=admin&id=1');
	return 0;
}



//process get requests
//PRE
//POST	1	success
function install_get()
{
	if(readfile('html/doctype.html') == FALSE)
		print("<html>\n");
	if(readfile('html/install.html') == FALSE)
		print("\t<head>
\t\t<title>DaPortal configuration</title>
\t</head>
\t<body>
\t\t<p>
\t\t\t<b>Error:</b> could not include \"html/install.html\".
\t\t</p>\n");
	else
		print("\t<body>\n");
	print("\t</body>
</html>\n");
	return 1;
}


//process post requests
//PRE
//POST	0	success
//	1	error
function install_post()
{
	global $connection, $dbtype, $dbhost, $dbport, $dbname, $dbuser,
	       $dbpassword;

	require_once('./system/debug.php');
	if(!get_magic_quotes_gpc())
		exit(_error('Magic quotes must be enabled'));
	$vars = array('dbtype', 'dbhost', 'dbport', 'dbname', 'dbuser',
			'dbpassword', 'template', 'theme');
	foreach($vars as $v)
		$$v = isset($_POST[$v]) ? stripslashes($_POST[$v]) : FALSE;
	if(!is_writable('.'))
	{
		if(readfile('html/doctype.html') == FALSE)
			print("<html>\n");
		print("\t<head>
\t\t<title>DaPortal processing configuration</title>
\t</head>
\t<body>
\t<h3>Directory is not writable</h3>
\t<p>
\t\tThe installation process has to modify \"index.php\" and create \"config.php\" at its installation root to complete. Please give the webserver the appropriate rights (and don't forget to secure permissions after the install).
\t</p>
</html>\n");
		return 1;
	}
	$_SERVER['SCRIPT_NAME'] = '/index.php'; //XXX
	return install_process($dbtype, $dbhost, $dbport, $dbname, $dbuser,
			$dbpassword, $template, $theme);
}


//process invalid requests
//PRE
//POST	exits with error
function install_invalid()
{
	readfile('html/doctype.html');
	readfile('html/invalid.html');
	exit(1);
}


//main
$html = 1;
if($_SERVER['REMOTE_ADDR'] != '127.0.0.1')
{
	if(readfile('html/doctype.html') != 0)
		print("<html>\n");
	print("\t<head>
\t\t<title>Server maintainance</title>
\t</head>
\t<body>
\t\t<h1>Server maintainance</h1>
\t\t<p>
\t\t\tSorry, the server is currently closed for maintainance.
\t\t</p>
\t\t<p>
\t\t\tIf you are the administrator of this site, please try it on the local interface of its host.
\t\t</p>\n");
	return 1;
}
if($_SERVER['REQUEST_METHOD'] == 'POST')
	return install_post();
if($_SERVER['REQUEST_METHOD'] != 'GET')
	return install_invalid();
return install_get();


?>
