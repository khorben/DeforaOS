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



$title = "<a href=\"index.php?module=news\">News</a>";


$content = "<ul>\n";

if(($res = sql_query("select newsid, title from daportal_news, daportal_contents where enable='1' and newsid=contentid order by date desc limit 3;")) != FALSE)
{
	while(sizeof($res) >= 1)
	{
		$content .= "\t<li><a href=\"index.php?module=news&amp;id=".$res[0]["newsid"]."\">".$res[0]["title"]."</a></li>\n";
		array_shift($res);
	}
}

$content .= "</ul>\n";
