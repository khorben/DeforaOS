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
if(eregi('install.php', $_SERVER['REQUEST_URI']))
{
	header('Location: index.php');
	exit(1);
}

require_once('system/raw.php');


//installing functions
//install_database
//resets and creates default tables
//PRE
//POST
function install_database()
{
	sql_table_drop('daportal_sessions');
	sql_table_drop('daportal_users');
	sql_table_drop('daportal_layouts');
	sql_table_drop('daportal_contents');
	sql_table_drop('daportal_modules');
	sql_sequence_drop('daportal_users_userid_seq');
	sql_sequence_drop('daportal_modules_moduleid_seq');
	sql_sequence_drop('daportal_contents_contentid_seq');

	sql_sequence_create('daportal_users_userid_seq');
	sql_sequence_create('daportal_modules_moduleid_seq');
	sql_sequence_create('daportal_contents_contentid_seq');
	//FIXME move users and sessions to module users,
	//and set up login procedure depending on module users enabled
	sql_table_create('daportal_users', "(
	userid integer DEFAULT nextval('daportal_users_userid_seq'),
	username varchar(9) NOT NULL UNIQUE,
	password varchar(32) NOT NULL,
	moderator bool NOT NULL DEFAULT '0',
	administrator bool NOT NULL DEFAULT '0',
	theme varchar(9) NOT NULL DEFAULT '',
	email varchar(60) NOT NULL DEFAULT '',
	homepage varchar(60) NOT NULL DEFAULT '',
	PRIMARY KEY (userid)
)");
	sql_table_create('daportal_sessions', "(
	sessionid varchar(32),
	userid integer,
	ip varchar(15) NOT NULL,
	expires date NOT NULL,
	FOREIGN KEY (userid) REFERENCES daportal_users (userid)
)");
	sql_table_create('daportal_modules', "(
	moduleid integer DEFAULT nextval('daportal_modules_moduleid_seq'),
	modulename varchar(9) NOT NULL UNIQUE,
	enable bool NOT NULL DEFAULT '1',
	PRIMARY KEY (moduleid)
)");
	sql_table_create('daportal_contents', "(
	contentid integer DEFAULT nextval('daportal_contents_contentid_seq'),
	moduleid integer,
	title varchar(80) NOT NULL DEFAULT '',
	content text NOT NULL,
	enable bool NOT NULL DEFAULT '0',
	PRIMARY KEY (contentid),
	FOREIGN KEY (moduleid) REFERENCES daportal_modules (moduleid)
)");

	sql_table_create('daportal_layouts', "(
	moduleid integer,
	actionid varchar(9) NOT NULL DEFAULT '',
	top varchar(9) NOT NULL DEFAULT '',
	bottom varchar(9) NOT NULL DEFAULT '',
	FOREIGN KEY (moduleid) REFERENCES daportal_modules (moduleid)
)");

	sql_query("insert into daportal_users (username, password, administrator) values ('admin', '5f4dcc3b5aa765d61d8327deb882cf99', '1');");

	require_once('system/module.php');
	module_install('admin');
	module_install('user');
	module_install('pages');
	module_install('news');
	return 0;
}

function install_config($dbtype, $dbhost, $dbport, $dbname, $dbuser, $dbpassword)
{
	if(($fp = fopen('config.php', 'w')) == FALSE)
		return 1;
	fwrite($fp, "<?php



//check url
if(eregi(\"config.php\", \$_SERVER[\"REQUEST_URI\"]))
{
	header(\"Location: index.php\");
	exit(0);
}

\$dbtype = \"$dbtype\";
\$dbhost = \"$dbhost\";
\$dbport = \"$dbport\";
\$dbname = \"$dbname\";
\$dbuser = \"$dbuser\";
\$dbpassword = \"$dbpassword\";


?>\n");
	fclose($fp);
	return 0;
}

function install_index()
{
	if(($fp = fopen('index.php', 'w')) == FALSE)
		return 1;
	fwrite($fp, "<?php



//check url
if(!eregi('index.php', \$_SERVER['REQUEST_URI']))
{
	header('Location: index.php');
	exit(0);
}

include('engine.php');


?>\n");
	fclose($fp);
	return 0;
}

//
//PRE
//POST	1	error
//	0	success
function install_process($dbtype, $dbhost, $dbport, $dbname, $dbuser, $dbpassword)
{
	if(install_database() != 0
			|| install_config($dbtype, $dbhost, $dbport,
					$dbname, $dbuser, $dbpassword) != 0
			|| install_index() != 0)
		return 1;
	header('Location: index.php');
	return 0;
}



//process get requests
//PRE
//POST	1	success
function install_get()
{
	if(raw_include('html/xhtml.html') != 0)
		print("<html>\n");
	if(raw_include('html/install.html') != 0)
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
	$dbtype = $_POST["dbtype"];
	$dbhost = $_POST["dbhost"];
	$dbport = $_POST["dbport"];
	$dbuser = $_POST["dbuser"];
	$dbpassword = $_POST["dbpassword"];
	$dbname = $_POST["dbname"];

	require_once("system/sql.php");
	//FIXME postgresql only
	//FIXME check input (in sql_connect?)
	if(sql_connect($dbhost, $dbport, $dbname, $dbuser, $dbpassword) != 0)
	{
		if(raw_include("html/xhtml.html") != 0)
			print("<html>\n");
		print("\t<head>
\t\t<title>DaPortal processing configuration</title>
\t</head>
\t<body>
\t<h3>Connection to the database failed</h3>
\t<p>
\t\t<b>Summary of your settings:</b><br/>
\t\t<b>Server type</b>: ");
		switch($dbtype)
		{
			case "pgsql":
				print("PostgreSQL<br/>\n");
				break;
			default:
				print("UNKNOWN<br/>\n");
				break;
		}
		print("\t\t<b>Hostname</b>: $dbhost<br/>
\t\t<b>Port</b>: $dbport<br/>
\t\t<b>Username</b>: $dbuser<br/>
\t\t<b>Password</b>: (hidden)<br/>
\t\t<b>Database</b>: $dbname<br/>
\t</p>
\t</body>
</html>\n");
		return 1;
	}
	if(!is_writable('.'))
	{
		if(raw_include('html/xhtml.html') != 0)
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
	return install_process($dbtype, $dbhost, $dbport, $dbname, $dbuser, $dbpassword);
}


//process invalid requests
//PRE
//POST	exits with error
function install_invalid()
{
	raw_require("html/xhtml.html");
	raw_require("html/invalid.html");
	exit(1);
}


//process requests
if($_SERVER["REMOTE_ADDR"] != "127.0.0.1")
{
	if(raw_include("html/xhtml.html") != 0)
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
global $administrator;
$administrator = 1;
if($_SERVER["REQUEST_METHOD"] == "POST")
	return install_post();
if($_SERVER["REQUEST_METHOD"] != "GET")
	return install_invalid();
return install_get();


?>
