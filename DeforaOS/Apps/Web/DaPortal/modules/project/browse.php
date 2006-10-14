<?php //modules/project/browse.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function _browse_dir($id, $project, $cvsrep, $cvsroot, $filename)
{
	print('<h1 class="project">'._html_safe($project).' CVS: '
			._html_safe($filename).'</h1>'."\n");
	//FIXME un-hardcode locations (invoke the cvs executable instead?)
	$path = $cvsrep.$cvsroot.'/'.$filename;
	if(($dir = opendir($path)) == FALSE)
		return _error('Could not open CVS repository', 1);
	$dirs = array();
	$files = array();
	while($de = readdir($dir))
	{
		if($de == '.' || $de == '..')
			continue;
		if(is_dir($path.'/'.$de))
			$dirs[] = $de;
		else
			$files[] = $de;
	}
	closedir($dir);
	sort($dirs);
	sort($files);
	$entries = array();
	foreach($dirs as $d)
	{
		$name = _html_safe_link($d);
		$name = '<a href="index.php?module=project&amp;action=browse'
				.'&amp;id='.$id.'&amp;file='.$filename.'/'
				.$name.'">'.$name.'</a>';
		$entries[] = array('name' => $name,
				'icon' => 'icons/16x16/mime/folder.png',
				'thumbnail' => 'icons/48x48/mime/folder.png');
	}
	foreach($files as $f)
	{
		unset($rcs);
		exec('rlog "'.str_replace('"', '\"', $path.'/'.$f).'"', $rcs);
		_info('rlog "'.str_replace('"', '\"', $path.'/'.$f).'"', 0);
		for($i = 0, $count = count($rcs); $i < $count; $i++)
			_info($i.': '.$rcs[$i], 0);
		for($revs = 0; $revs < $count; $revs++)
			if($rcs[$revs] == '----------------------------')
				break;
		$file = _html_safe_link($filename.'/'.$f);
		$name = _html_safe(substr($rcs[2], 14));
		require_once('./system/mime.php');
		$mime = _mime_from_ext($name);
		$name = '<a href="index.php?module=project&amp;action=browse'
				.'&amp;id='.$id.'&amp;file='.$file.'">'
				.$name.'</a>';
		$thumbnail = is_readable('./icons/48x48/mime/'.$mime
				.'.png')
			? 'icons/48x48/mime/'.$mime.'.png'
			: 'icons/48x48/mime/default.png';
		$icon = is_readable('./icons/16x16/mime/'.$mime.'.png')
			? 'icons/16x16/mime/'.$mime.'.png'
			: $thumbnail;
		$revision = _html_safe(substr($rcs[$revs+1], 9));
		$revision = '<a href="index.php?module=project'
				.'&amp;action=browse&amp;id='.$id
				.'&amp;file='.$file.'&amp;revision='.$revision
				.'">'.$revision.'</a>';
		$date = _html_safe(substr($rcs[$revs+2], 6, 19));
		$author = substr($rcs[$revs+2], 36);
		$author = substr($author, 0, strspn($author,
				'abcdefghijklmnopqrstuvwxyz'
				.'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
				.'0123456789'));
		require_once('./system/user.php');
		if(($author_id = _user_id($author)) != FALSE)
		{
			$author = _html_safe_link($author);
			$author = '<a href="index.php?module=user&amp;id='
					.$author_id.'">'.$author.'</a>';
		}
		else
			$author = '';
		//FIXME this is certainly variable (number of lines)
		$message = _html_safe($rcs[$revs+3]);
		//FIXME choose icon depending on the file type
		$entries[] = array('name' => $name,
				'icon' => $icon,
				'thumbnail' => $thumbnail,
				'revision' => $revision,
				'date' => $date,
				'author' => $author,
				'message' => $message);
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'Back', 'icon' => 'icons/16x16/back.png',
			'link' => 'javascript:history.back()');
	$toolbar[] = array('title' => 'Parent directory',
			'icon' => 'icons/16x16/updir.png',
			'link' => 'index.php?module=project&action=browse'
					.'&id='.$id
					.(strlen($filename)
					? '&file='.dirname($filename) : ''));
	$toolbar[] = array('title' => 'Forward',
			'icon' => 'icons/16x16/forward.png',
			'link' => 'javascript:history.forward()');
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Refresh',
			'icon' => 'icons/16x16/refresh.png',
			'link' => 'javascript:location.reload()');
	_module('explorer', 'browse_trusted', array('entries' => $entries,
			'class' => array('revision' => 'Revision',
					'date' => 'Date',
					'author' => AUTHOR,
					'message' => MESSAGE),
			'view' => 'details',
			'toolbar' => $toolbar));
}

function _browse_file($id, $project, $cvsrep, $cvsroot, $filename)
	//FIXME
	//- allow diff requests
	//also think about:
	//- creating archives
{
	$path = $cvsrep.$cvsroot.'/'.$filename;
	$path = str_replace('"', '\"', $path);
	$path = str_replace('$', '\$', $path);
	exec('rlog "'.$path.'"', $rcs);
	_info('rlog "'.$path.'"', 0);
	print('<h1 class="project">'._html_safe($project).' CVS: '
			._html_safe(dirname($filename)).'/'
			._html_safe(substr($rcs[2], 14)).'</h1>'."\n");
	for($i = 0, $count = count($rcs); $i < $count; $i++)
		_info($i.': '.$rcs[$i], 0);
	for($i = 0; $i < $count;)
		if($rcs[$i++] == '----------------------------')
			break;
	$revisions = array();
	for($count = count($rcs); $i < $count; $i+=3)
	{
		$name = _html_safe(substr($rcs[$i], 9));
		$date = _html_safe(substr($rcs[$i+1], 6, 19));
		$author = substr($rcs[$i+1], 36);
		$author = substr($author, 0, strspn($author,
				'abcdefghijklmnopqrstuvwxyz'
				.'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
				.'0123456789'));
		require_once('./system/user.php');
		if(($author_id = _user_id($author)) != FALSE)
		{
			$author = _html_safe_link($author);
			$author = '<a href="index.php?module=user&amp;id='
					.$author_id.'">'.$author.'</a>';
		}
		else
			$author = '';
		$message = $rcs[$i+2];
		if($message == '----------------------------'
				|| $message ==
'=============================================================================')
			$message = '';
		else
		{
			$apnd = '';
			for($i++; $i < $count
					&& $rcs[$i+2] !=
					'----------------------------'
					&& $rcs[$i+2] !=
'=============================================================================';
					$i++)
				$apnd = '...';
			$message.=$apnd;
			$message = _html_safe($message);
		}
		$icon = (strcmp($name, '1.1') == 0)
			? 'modules/project/cvs-added.png'
			: 'modules/project/cvs-modified.png';
		$revisions[] = array('module' => 'project',
				'action' => 'browse',
				'id' => $id,
				'args' => '&file='.$filename.'&revision='.$name,
				'icon' => $icon,
				'thumbnail' => $icon,
				'name' => $name,
				'date' => $date,
				'author' => $author,
				'message' => $message);
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'Back', 'icon' => 'icons/16x16/back.png',
			'link' => 'javascript:history.back()');
	$toolbar[] = array('title' => 'Parent directory',
			'icon' => 'icons/16x16/updir.png',
			'link' => 'index.php?module=project&action=browse'
					.'&id='.$id
					.'&file='.dirname($filename));
	$toolbar[] = array('title' => 'Forward',
			'icon' => 'icons/16x16/forward.png',
			'link' => 'javascript:history.forward()');
	$toolbar[] = array();
	$toolbar[] = array('title' => 'Refresh',
			'icon' => 'icons/16x16/refresh.png',
			'link' => 'javascript:location.reload()');
	_module('explorer', 'browse_trusted', array('entries' => $revisions,
			'class' => array('date' => 'Date',
					'author' => AUTHOR,
					'message' => MESSAGE),
			'toolbar' => $toolbar,
			'view' => 'details'));
}

function _browse_file_revision($id, $project, $cvsrep, $cvsroot, $filename,
		$revision, $download)
{
	if(!ereg('^[0-9]+\.[0-9]+$', $revision))
		return _error('Invalid revision');
	$path = $cvsrep.$cvsroot.'/'.$filename;
	$path = str_replace('"', '\"', $path);
	$path = str_replace('$', '\$', $path);
	exec('rlog -h "'.$path.'"', $rcs);
	_info('rlog -h "'.$path.'"', 0);
	for($i = 0, $count = count($rcs); $i < $count; $i++)
		_info($i.': '.$rcs[$i], 0);
	if(!$download)
		print('<h1 class="project">'._html_safe($project).' CVS: '
				._html_safe(dirname($filename)).'/'
				._html_safe(substr($rcs[2], 14))
				.' '.$revision.'</h1>'."\n");
	$basename = substr($rcs[2], 14);
	$fp = popen('co -p'.$revision.' "'.$path.'"', 'r');
	_info('co -p'.$revision.' "'.$path.'"', 0);
	require_once('./system/mime.php');
	if(($mime = _mime_from_ext($basename)) == 'default')
		$mime = 'text/plain';
	if($download)
	{
		require_once('./system/html.php');
		header('Content-Type: '.$mime);
		header('Content-Disposition: inline; filename="'
				._html_safe(basename($basename)).'"');
		while(!feof($fp))
			print(fread($fp, 8192));
		return;
	}
	$link = "index.php?module=project&amp;action=browse&amp;id=$id"
		."&amp;file="._html_safe_link($filename);
	print('<div class="toolbar"><a href="'.$link.'"'
			.' title="Browse revisions">'
			.'<img src="icons/16x16/updir.png" alt=""/>'
			.' Browse revisions</a>'
			.' &middot; <a href="'.$link."&amp;revision=$revision"
			.'&amp;download=1" title="Download file">'
			.'<img src="icons/16x16/save.png" alt=""/>'
			.' Download file</a></div>'."\n");
	if(strncmp('image/', $mime, 6) == 0)
		return print('<pre><img src="'.$link.' alt=""/></pre>'."\n");
	include('syntax.php');
	print('<pre>'."\n");
	switch($mime)
	{
		case 'application/x-php':
			while(!feof($fp))
				print(_file_php(_html_safe(fgets($fp, 8192))));
			break;
		case 'text/x-chdr':
		case 'text/x-csrc':
			while(!feof($fp))
				print(_file_csrc(_html_safe(fgets($fp, 8192))));
			break;
		case 'text/x-makefile':
			while(!feof($fp))
				print(_file_makefile(_html_safe(fgets($fp, 8192))));
			break;
		default:
			while(!feof($fp))
				print(_html_safe(fgets($fp, 8192)));
			break;
	}
	print('</pre>'."\n");
}

?>
