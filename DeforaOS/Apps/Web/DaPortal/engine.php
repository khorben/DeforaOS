<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//global variables
$debug = 0;
$friendlylinks = 0;
$friendlykicker = 'index.php';
$html = 1;
$template = 'DaPortal';
$theme = 'DaPortal';
require_once('./config.php');

//internals
$user_id = 0;
$user_name = 'Anonymous';
$module_id = 0;
$module_name = '';

//new engine
if(chdir('./src') != TRUE)
	exit(2);
require_once('./system/config.php');
global $config;
$config = new Config();
$config->load('../daportal.conf');
require_once('./engines/daportal.php');
$engine = new DaPortalEngine;
$engine->attach();
if(($db = $engine->getDatabase()) === FALSE)
{
	print('Could not connect to the database');
	exit(2);
}
$engine->getAuth();
if(chdir('..') != TRUE)
	exit(2);

//mandatory code
require_once('./system/debug.php');
if(isset($_COOKIE[session_name()]))
{
	$vars = array_keys($_SESSION);
	foreach($vars as $v)
		$$v = $_SESSION[$v];
}
//FIXME obtain credentials from DaPortalEngine
require_once('./system/sql.php');
require('./system/module.php');
require('./system/config.php');
require_once('./system/lang.php');

//configuration variables
if(!isset($title) && ($title = _config_get('admin', 'title')) == FALSE)
	$title = 'DaPortal';
if(!isset($_SESSION['theme']) && ($t = _config_get('admin', 'theme')) != FALSE)
	$theme = $t;

//parse url
if($friendlylinks == 1 && $_SERVER['REQUEST_METHOD'] == 'GET'
		&& isset($_SERVER['PATH_INFO'])
		&& _module_parse_friendly($_SERVER['PATH_INFO']) != TRUE)
{
	header('HTTP/1.0 404 Not Found');
	readfile('./html/doctype.html');
	readfile('./html/404.html');
	exit(0);
}

_module(FALSE, 'system');

if($html)
{
	require_once('./system/html.php');
	_html_start();
	_html_template($template);
	_html_stop();
}
else
	_module();

?>
