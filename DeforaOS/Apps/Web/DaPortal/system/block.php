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
if(eregi("block.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: ../index.php");
	exit(1);
}


function block_include($block)
{
	global $title, $content;

	$title = "";
	$content = "";
	if($block == "index")
		return 1;
	if(!ereg("^[a-z]+$", $block))
		return 1;
	$block = "blocks/".$block.".php";
	if(!include($block))
	{
		print("\t\t<b>Warning:</b> could not include a block.<br/>\n");
		return 1;
	}
	print("\t\t<div class=\"block\">
\t\t\t<div class=\"block_title\">
\t\t\t\t$title
\t\t\t</div>
\t\t\t<div class=\"block_content\">
$content
\t\t\t</div>
\t\t</div>\n");

	return 1;
}


?>
