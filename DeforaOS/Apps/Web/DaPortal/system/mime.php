<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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



function _mime_from_ext($filename)
{
	static $globfile = 0;
	static $globs = array();

	if($globfile == 0 && is_readable('/usr/share/mime/globs'))
	{
		if(($globfile = file_get_contents('/usr/share/mime/globs'))
				== FALSE)
			return 'default';
		$globfile = explode("\n", $globfile);
		array_shift($globfile);
		array_shift($globfile);
		foreach($globfile as $l)
			$globs[] = explode(':', $l);
	}
	foreach($globs as $g)
		if(isset($g[1]) && fnmatch($g[1], $filename))
			return $g[0];
	return 'default';
}

?>
