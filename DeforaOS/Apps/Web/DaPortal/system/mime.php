<?php //system/mime.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _mime_from_ext($filename)
{
	static $globfile = 0;
	static $globs;

	_info($filename);
	if($globfile == 0)
	{
		if(($globfile = file_get_contents('/usr/share/mime/globs'))
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
