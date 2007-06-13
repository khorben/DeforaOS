<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
//Some parts Copyright (c) 2005 FPconcept (used with permission)
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



function filename_safe($filename)
{
	while($filename != ($filename = ereg_replace('(^|/)\.{1,2}($|/)', '/',
					$filename)));
	return $filename;
}


function html_safe($text)
{
	return htmlentities($text);
}


function html_safe_link($text)
{
	return str_replace('&amp;', '%26', htmlentities($text));
}


function mime_from_ext($filename)
{
	static $globfile = 0;
	static $globs;
	
	if($globfile == 0)
	{
		if(($globfile = @file_get_contents('/usr/share/mime/globs'))
				== FALSE)
			return 'default';
		$globfile = explode("\n", $globfile);
		array_shift($globfile);
		array_shift($globfile);
		$globs = array();
		foreach($globfile as $l)
			$globs[] = explode(':', $l);
	}
	foreach($globs as $g)
		if(fnmatch($g[1], $filename))
			return $g[0];
	return 'default';
}

?>
