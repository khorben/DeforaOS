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



$title = "Login";

global $userid, $username, $administrator;
if($userid == 0)
	$content = "<form method=\"post\" action=\"index.php\">
\t<div>
\t\tUsername:<br/><input type=\"text\" name=\"username\" size=\"8\"/><br/>
\t\tPassword:<br/><input type=\"password\" name=\"password\" size=\"8\"/><br/>
\t\t<input type=\"submit\" value=\"Login\"/>
\t\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t\t<input type=\"hidden\" name=\"action\" value=\"login\"/>
\t\t<input type=\"hidden\" name=\"query\" value=\"".$_SERVER['QUERY_STRING']."\"/>
\t</div>
</form>
<div>
\t<a href=\"index.php?module=user\">Register</a>
</div>\n";
else
{
	$content = "Logged in as <a href=\"index.php?module=user&action=homepage\">$username</a><br/>
<br/>\n";
	if($administrator == 1)
	{
		$content .= "<a href=\"index.php?module=admin\">Administration</a><br/>\n";
		if($_GET["module"] != "")
			$content .= "<a href=\"index.php?module=".$_GET["module"]."&amp;action=admin\">Module ".$_GET["module"]."</a><br/>\n";
		else
			$content .= "<a href=\"index.php?module=pages&amp;action=edit&amp;page=".$_GET["page"]."\">Edit page</a><br/>\n";
		$content .= "<br/>\n";
	}
	$content .= "<form method=\"post\" action=\"index.php\">
\t<div>
\t\t<input type=\"submit\" value=\"Logout\"/>
\t\t<input type=\"hidden\" name=\"module\" value=\"user\"/>
\t\t<input type=\"hidden\" name=\"action\" value=\"logout\"/>
\t</div>
</form>";
}


?>
