<?php
//engine.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: index.php'));


//global variables
$debug = 1;
$html = 1;
$title = 'DaPortal';
$template = 'DaPortal';
$theme = 'DaPortal';
require_once('config.php');

//internals
$user_id = 0;
$user_name = 'Anonymous';
$module_id = 0;
$module_name = '';

//mandatory code
//FIXME force magic quotes
session_start();
$vars = array_keys($_SESSION);
foreach($vars as $v)
	$$v = $_SESSION[$v];
require_once('system/debug.php');
require_once('system/sql.php');
require_once('system/module.php');
if($html)
{
	require_once('system/html.php');
	_html_start();
	_html_template($template);
	_html_stop();
}
else
	_module();

?>
