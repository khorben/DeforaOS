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
if(eregi('action.php', $_SERVER['REQUEST_URI']))
{
	header('Location: ../../index.php');
	exit(1);
}
require_once('module.php');

switch($action)
{
	case 'admin':
		return news_admin();
	case 'dump':
		return news_dump();
	case 'install':
		return news_install();
	case 'last':
		return news_last(1);
	case 'last2':
		return news_last(2);
	case 'last3':
		return news_last(3);
	case 'moderate':
		return news_moderate();
	case 'propose':
		return news_propose();
	case 'submit':
		return news_submit();
	case 'thanks':
		return news_thanks();
	case 'uninstall':
		return news_uninstall();
	default:
		return news_default();
}


?>
