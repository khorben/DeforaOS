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
	header('Location: ../index.php');
	exit(1);
}


function module_cache_include($module, $cache)
{
	return raw_include("modules/$module/$cache.html");
}


function module_id($module, $enable)
{
	$query = "select moduleid, modulename from daportal_modules where modulename='$module'";
	if($enable == 0 || $enable == 1)
		$query .= " and enable='$enable'";
	if(($res = sql_query($query)) == FALSE)
		return 0;
	return $res[0]['moduleid'];
}


function module_name($moduleid, $enable)
{
	$query = "select modulename from daportal_modules where moduleid='$moduleid'";
	if($enable == 0 || $enable == 1)
		$query .= " and enable='$enable'";
	if(($res = sql_query($query)) == FALSE)
		return FALSE;
	return $res[0]['modulename'];
}


function module_include($module, $action)
{
	global $moduleid;

	if(($moduleid = module_id($module, 1)) == 0)
	{
		print("\t\t<div><b>Warning:</b> could not include a module (unknown).</div>\n");
		return 1;
	}
	if(module_cache_include($module, $action) == 0)
		return 0;
	if(module_process($module, $action) != 0)
	{
		print("\t\t<div><b>Warning:</b> could not include a module (module error).</div>\n");
		return 1;
	}
	return 0;
}


//PRE	administrator privileges are checked
//	$module is trusted
function module_install($module)
{
	sql_query("insert into daportal_modules (modulename, enable) values ('$module', '1');");
	$res = sql_query("select moduleid from daportal_modules where modulename='$module';");
	global $moduleid;
	$moduleid = $res[0]['moduleid'];
	$action = 'install';
	return include('modules/'.$module.'/action.php');
}


function module_uninstall($module)
{
	$action = 'uninstall';
	include('modules/'.$module.'/action.php');
	sql_query("delete from daportal_modules where modulename='$module';");
}


//PRE	$module must be valid
function module_process($module, $action)
{
	if($action == 'install' || $action == 'uninstall')
		$action = 'default';
	return include('modules/'.$module.'/action.php');
}


?>
