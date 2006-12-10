<?php //system/mime.php



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
