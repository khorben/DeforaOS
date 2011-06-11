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
$html = 1;
$template = 'DaPortal';
$theme = 'DaPortal';
require_once('./config.php');

//internals
$user_id = 0;
$user_name = 'Anonymous';
$module_id = 0;
$module_name = '';

//mandatory code
require_once('./system/debug.php');
if(!get_magic_quotes_gpc())
	exit(_error('Magic quotes must be enabled'));
if(isset($_COOKIE[session_name()]))
{
	session_start();
	$vars = array_keys($_SESSION);
	foreach($vars as $v)
		$$v = $_SESSION[$v];
}
require_once('./system/sql.php');
require_once('./system/module.php');
require_once('./system/config.php');

//configuration variables
if(!isset($title) && ($title = _config_get('admin', 'title')) == FALSE)
	$title = 'DaPortal';
if(!isset($_SESSION['theme']) && ($t = _config_get('admin', 'theme')) != FALSE)
	$theme = $t;

require_once('./system/lang.php');

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

_module('', 'system');

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
