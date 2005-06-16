<?php
//Copyright 2005 Pierre Pronchery
//Some parts Copyright 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//PHP Explorer is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
//PHP Explorer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PHP Explorer; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require('common.php');
require('config.php');


$folder = filename_safe($_GET['folder']);
if(!$upload)
	$message = '';
else if(isset($_POST['newdir']))
{
	global $root;

	$newdir = filename_safe(stripslashes($_POST['folder']))
			.'/'.filename_safe(stripslashes($_POST['newdir']));
	if(@mkdir($root.'/'.$newdir) == FALSE)
		$message = 'Could not create directory "'.$newdir.'"';
	else
		$message = 'Directory "'.$newdir.'" successfully created';
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>Create directory</title>
		<link type="text/css" rel="stylesheet" href="explorer.css"/>
		<script type="text/javascript" src="explorer.js"></script>
	</head>
	<body style="margin: 0; padding: 5px">
<? if(strlen($message)) { ?>
		<h3>Directory creation</h3>
		<p><? echo html_safe($message); ?></p>
		<center><input type="button" value="Close" onclick="window.close()"/></center>
<? } else { ?>
		<h3>Create directory</h3>
		<p>The new directory will be created in folder:<br/><i><? echo html_safe($folder); ?></i></p>
		<form action="newdir.php" method="post">
			<input type="hidden" name="folder" value="<? echo html_safe($folder); ?>"/>
			<table>
				<tr><td><b>Directory name:</b></td><td><input type="text" name="newdir" size="20"/></td></tr>
				<tr><td align="center"><input type="button" value="Cancel" onclick="window.close()"/></td><td align="center"><input type="submit" value="Create"/></td></tr>
			</table>
		</form>
<? } ?>
	</body>
</html>
