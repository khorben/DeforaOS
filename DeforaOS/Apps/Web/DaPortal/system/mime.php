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
	static $types = FALSE;

	if(!is_array($types))
	{
		$types = array();
		if(($globs = _config_get('admin', 'globs')) == FALSE)
		{
			_error('MIME globs file is not defined', 0);
			return 'default';
		}
		if(($globs = file_get_contents($globs)) == FALSE)
		{
			_error('Could not read MIME globs file', 0);
			return 'default';
		}
		$globs = explode("\n", $globs);
		array_shift($globs);
		array_shift($globs);
		foreach($globs as $l)
			$types[] = explode(':', $l);
	}
	foreach($types as $g)
		if(isset($g[1]) && fnmatch($g[1], $filename))
			return $g[0];
	return 'default';
}

?>
