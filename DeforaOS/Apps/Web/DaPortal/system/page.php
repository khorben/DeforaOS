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
if(eregi("page.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../index.php");
	exit(1);
}


function page_include($page)
{
	if(($fp = @fopen("pages/$page.tpl", "r")) == NULL)
	{
		print("<b>Warning:</b> could not open a page<br/>\n");
		return 1;
	}
	while(!feof($fp))
	{
		$line = fgets($fp, 8192);
		if(ereg("^BLOCK: ([a-z]+)\r\n$", $line, $args))
		{
			require_once("system/block.php");
			block_include($args[1]);
		}
		else if(ereg("^MODULE: ([a-z]{1,9})( ([a-z]{1,9}))?\r\n$", $line, $args))
		{
			require_once("system/module.php");
			module_include($args[1], $args[3]);
		}
		else if(ereg("^PAGE: ([a-z]{1,9})\r\n$", $line, $args))
			page_include($args[1]);
		else
			print("$line");
	}
	return 0;
}
