<?php //modules/project/module.php
//FIXME license
//FIXME hide attic option



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['BROWSE_SOURCE'] = 'Browse source';
$text['BUG_REPORTS'] = 'Bug reports';
$text['CVS_PATH'] = 'CVS path';
$text['INVALID_PROJECT'] = 'Invalid project';
$text['MEMBERS'] = 'Members';
$text['NEW_PROJECT'] = 'New project';
$text['NO_CVS_REPOSITORY'] = 'This project does not have a CVS repository';
$text['PRIORITY'] = 'Priority';
$text['PROJECT'] = 'Project';
$text['PROJECT_LIST'] = 'Project list';
$text['PROJECT_NAME'] = 'Project name';
$text['PROJECTS'] = 'Projects';
$text['PROJECTS_ADMINISTRATION'] = 'Projects administration';
$text['REPLY_TO_BUG'] = 'Reply to bug';
$text['REPORT_A_BUG'] = 'Report a bug';
$text['REPORT_BUG_FOR'] = 'Report bug for';
$text['STATE'] = 'State';
$text['TIMELINE'] = 'Timeline';
global $lang;
if($lang == 'de')
{
	$text['BROWSE_SOURCE'] = 'Quellcode betrachten';
	$text['NEW_PROJECT'] = 'Neues projekt';
	$text['PRIORITY'] = 'Priorität';
	$text['PROJECT'] = 'Projekt';
	$text['PROJECT_LIST'] = 'Projektliste';
	$text['PROJECTS'] = 'Projekte';
	$text['STATE'] = 'Stand';
	$text['TIMELINE'] = 'Fortschritt';
}
else if($lang == 'fr')
{
	$text['BROWSE_SOURCE'] = 'Parcourir les sources';
	$text['BUG_REPORTS'] = 'Rapports de bugs';
	$text['INVALID_PROJECT'] = 'Projet non valide';
	$text['MEMBERS'] = 'Membres';
	$text['NEW_PROJECT'] = 'Nouveau projet';
	$text['NO_CVS_REPOSITORY'] = "Ce projet n'est pas géré par CVS";
	$text['PRIORITY'] = 'Priorité';
	$text['PROJECT'] = 'Projet';
	$text['PROJECT_LIST'] = 'Liste des projets';
	$text['PROJECT_NAME'] = 'Nom du projet';
	$text['PROJECTS'] = 'Projets';
	$text['PROJECTS_ADMINISTRATION'] = 'Administration des projets';
	$text['REPORT_A_BUG'] = 'Rapporter un bug';
	$text['REPORT_BUG_FOR'] = 'Rapporter un bug pour';
	$text['STATE'] = 'Etat';
	$text['TIMELINE'] = 'Progression';
}
_lang($text);


function _project_toolbar($id, $admin = 0)
{
	global $html;

	if(!$html)
		return;
	$cvsroot = '';
	$enabled = 0;
	$project = _sql_array('SELECT cvsroot, enabled'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='$id';");
	if(is_array($project) && count($project) == 1)
	{
		$cvsroot = $project[0]['cvsroot'];
		$enabled = $project[0]['enabled'] == 't' ? 1 : 0;
	}
	include('toolbar.tpl');
}


function _project_name($id)
{
	return _sql_single('SELECT name FROM daportal_project'
			." WHERE project_id='$id';");
}


function project_admin($args)
{
	global $user_id, $module_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return project_modify($args);
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe(PROJECTS_ADMINISTRATION).'</h1>'."\n");
	if(($configs = _config_list('project')))
	{
		print('<h2><img src="modules/project/icon.png" alt=""/>'
				.' Configuration</h2>'."\n");
		$module = 'project';
		$action = 'config_update';
		include('system/config.tpl');
	}
	print('<h2><img src="modules/project/icon.png" alt=""/> '
			._html_safe(PROJECT_LIST)
			.'</h2>'."\n");
	$projects = _sql_array('SELECT content_id AS id, name, title AS desc'
			.', username AS admin, daportal_content.enabled'
			.', daportal_content.user_id AS user_id, cvsroot'
			.' FROM daportal_content, daportal_user'
			.', daportal_project'
			.' WHERE daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id'
			.' ORDER BY name ASC;');
	if(!is_array($projects))
		return _error('Could not list projects');
	$count = count($projects);
	for($i = 0; $i < $count; $i++)
	{
		$projects[$i]['name'] = _html_safe_link($projects[$i]['name']);
		$projects[$i]['icon'] = 'modules/project/icon.png';
		$projects[$i]['thumbnail'] = 'modules/project/icon.png';
		$projects[$i]['admin'] = '<a href="index.php?module=project'
				.'&amp;action=list'
				.'&amp;user_id='.$projects[$i]['user_id'].'">'
				._html_safe_link($projects[$i]['admin']).'</a>';
		$projects[$i]['desc'] = _html_safe($projects[$i]['desc']);
		$projects[$i]['module'] = 'project';
		$projects[$i]['action'] = 'modify';
		$projects[$i]['apply_module'] = 'project';
		$projects[$i]['apply_id'] = $projects[$i]['id'];
		$projects[$i]['enabled'] = ($projects[$i]['enabled'] == 't')
			? 'enabled' : 'disabled';
		$projects[$i]['enabled'] = '<img src="icons/16x16/'
			.$projects[$i]['enabled'].'.png" alt="'
			.$projects[$i]['enabled'].'"/>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_PROJECT,
			'icon' => 'modules/project/icon.png',
			'link' => 'index.php?module=project&action=new');
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable',
			'confirm' => 'disable');
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable',
			'confirm' => 'enable');
	$toolbar[] = array('title' => DELETE,
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete',
			'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array(
			'class' => array('enabled' => 'Enabled',
					'admin' => ADMINISTRATOR,
					'desc' => DESCRIPTION,
					'cvsroot' => CVS_PATH),
			'toolbar' => $toolbar,
			'view' => 'details',
			'entries' => $projects,
			'module' => 'project',
			'action' => 'admin'));
}


function project_browse($args)
{
	$project = _sql_array('SELECT name, cvsroot'
			.' FROM daportal_content, daportal_project'
			." WHERE project_id='".$args['id']."'"
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id'
			." AND enabled='1';");
	if(!is_array($project) || count($project) != 1
			|| !($cvsrep = _config_get('project', 'cvsroot')))
		return _error(INVALID_PROJECT);
	$cvsrep.='/';
	$project = $project[0];
	_project_toolbar($args['id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($project['name']).' CVS</h1>'."\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	if(!ereg('^[a-zA-Z0-9. /]+$', $project['cvsroot'])
			|| ereg('\.\.', $project['cvsroot']))
		return _error('Invalid CVSROOT', 1);
	if(!isset($args['file']))
		return _browse_dir($args['id'], $project['name'], $cvsrep,
				$project['cvsroot'], '');
	$file = stripslashes($args['file']);
	if(!ereg('^[a-zA-Z0-9.,-_ /]+$', $file) || ereg('\.\.', $file))
		return _error('Invalid path', 1);
	$filename = $cvsrep.$project['cvsroot'].'/'.$file;
	if(is_dir($filename))
		return _browse_dir($args['id'], $project['name'], $cvsrep,
				$project['cvsroot'], $file);
	else if(file_exists($filename))
	{
		if(isset($args['revision']))
			return _browse_file_revision($args['id'],
					$project['name'], $cvsrep,
					$project['cvsroot'],
					$file, $args['revision'],
					$args['download'] == 1);
		return _browse_file($args['id'], $project['name'], $cvsrep,
				$project['cvsroot'], $file);
	}
	_error('Invalid filename: "'.$filename.'"', 0);
	_browse_dir($args['id'], $project['name'], $cvsrep, $project['cvsroot'],
			'');
}

function _browse_dir($id, $project, $cvsrep, $cvsroot, $filename)
{
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($project).' CVS: '
			._html_safe($filename).'</h1>'."\n");
	//FIXME un-hardcode locations (invoke the cvs executable instead?)
	$path = $cvsrep.$cvsroot.'/'.$filename;
	if(($dir = @opendir($path)) == FALSE)
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
		require_once('system/mime.php');
		$mime = _mime_from_ext($name);
		$name = '<a href="index.php?module=project&amp;action=browse'
				.'&amp;id='.$id.'&amp;file='.$file.'">'
				.$name.'</a>';
		$thumbnail = is_readable('icons/48x48/mime/'.$mime
				.'.png')
			? 'icons/48x48/mime/'.$mime.'.png'
			: 'icons/48x48/mime/default.png';
		$icon = is_readable('icons/16x16/mime/'.$mime.'.png')
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
		require_once('system/user.php');
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
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($project).' CVS: '
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
		require_once('system/user.php');
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
		$revisions[] = array('module' => 'project',
				'action' => 'browse',
				'id' => $id,
				'args' => '&file='.$filename.'&revision='.$name,
				'name' => $name,
				'date' => $date,
				'author' => $author,
				'message' => $message);
	}
	_module('explorer', 'browse_trusted', array('entries' => $revisions,
			'class' => array('date' => 'Date',
					'author' => AUTHOR,
					'message' => MESSAGE),
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
		print('<h1><img src="modules/project/icon.png" alt=""/> '
				._html_safe($project).' CVS: '
				._html_safe(dirname($filename)).'/'
				._html_safe(substr($rcs[2], 14))
				.' '.$revision.'</h1>'."\n");
	$basename = substr($rcs[2], 14);
	$fp = popen('co -p'.$revision.' "'.$path.'"', 'r');
	_info('co -p'.$revision.' "'.$path.'"', 0);
	require_once('system/mime.php');
	if(($mime = _mime_from_ext($basename)) == 'default')
		$mime = 'text/plain';
	if($download)
	{
		require_once('system/html.php');
		header('Content-Type: '.$mime);
		header('Content-Disposition: inline; filename="'
				._html_safe(basename($basename)).'"');
		while(!feof($fp))
			print(fread($fp, 8192));
		return;
	}
	$link = "index.php?module=project&amp;action=browse&amp;id=$id"
		."&amp;file="._html_safe_link($filename)
		."&amp;revision=$revision&amp;download=1";
	print('<div class="toolbar"><a href="'.$link.'">Download file</a></div>'
			."\n");
	if(strncmp('image/', $mime, 6) == 0)
		return print('<pre><img src="'.$link.' alt=""/></pre>'."\n");
	print('<pre>'."\n");
	while(!feof($fp))
	{
		$line = _html_safe(fgets($fp, 8192));
		switch($mime)
		{
			case 'text/x-chdr':
			case 'text/x-csrc':
				$line = _file_csrc($line);
				break;
		}
		print($line);
	}
	print('</pre>'."\n");
}

function _file_csrc($line)
{
	static $comment = 0;

	$line = preg_replace('/(&quot;[^(&quot;)]+&quot;)/',
			'<span class="string">\1</span>',
			$line);
	if($line[0] == '#')
	{
		$line = '<span class="preprocessed">'.substr($line, 0, -1)
			.'</span>'."\n";
		return $line;
	}
	$line = preg_replace('/(^|[^a-zA-Z0-9])(break|case|continue|default'
				.'|do|else|for|if|return|switch'
				.'|until|while'
				.')($|[^a-zA-Z0-9])/',
			'\1<span class="keyword">\2</span>\3', $line);
	$line = preg_replace('/(^|[^a-zA-Z0-9])('
				.'char|double|FILE|float|int'
				.'|long|short|signed|static'
				.'|struct|typedef|unsigned|void'
				.')($|[^a-zA-Z0-9])/',
			'\1<span class="type">\2</span>\3', $line);
	/* FIXME fails on quoted strings, use prefix.line.suffix to escape hl */
	if($comment == 0 && strstr($line, '/*'))
	{
		$p = strpos($line, '/*');
		$line = substr($line, 0, $p).'<span class="comment">'
			.substr($line, $p);
		$comment = 1;
	}
	if($comment != 0 && strstr($line, '*/'))
	{
		$p = strpos($line, '*/');
		$line = substr($line, 0, $p+2).'</span>'
			.substr($line, $p+2);
		$comment = 0;
	}
	return $line;
}


function project_bug_display($args)
{
	global $user_id;

	if(!is_numeric($args['id']))
		return _error(INVALID_ARGUMENT);
	$bug = _sql_array('SELECT daportal_content.content_id AS content_id'
			.', daportal_project.project_id AS project_id'
			.', daportal_user.user_id AS user_id'
			.', daportal_bug.bug_id AS id, timestamp, title'
			.', content, name AS project, username'
			.', state, type, priority'
			.' FROM daportal_content, daportal_bug, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='t'"
			.' AND daportal_content.content_id'
			.'=daportal_bug.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_project.project_id'
			.'=daportal_bug.project_id'
			." AND bug_id='".$args['id']."';");
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug', 1);
	$bug = $bug[0];
	$title = 'Bug #'.$bug['id'].': '.$bug['title'];
	require_once('system/user.php');
	$admin = _user_admin($user_id) ? 1 : 0;
	include('bug_display.tpl');
	$replies = _sql_array('SELECT bug_reply_id AS id, title, content'
			.', state, type, priority, daportal_user.user_id'
			.', username'
			.' FROM daportal_bug_reply, daportal_content'
			.', daportal_user'
			.' WHERE daportal_bug_reply.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND (daportal_user.enabled='1' OR daportal_user.user_id='0')"
			." AND daportal_bug_reply.bug_id='".$bug['id']."';");
	if(!is_array($replies))
		return _error('Unable to display bug feedback');
	$cnt = count($replies);
	for($i = 0; $i < $cnt; $i++)
	{
		$reply = $replies[$i];
		include('bug_reply_display.tpl');
	}
}


function project_bug_insert($args)
{
	global $user_id;

	require_once('system/content.php');
	require_once('system/user.php');
	$enable = 0;
	if(_user_admin($user_id))
		$enable = 1;
	if(($id = _content_insert($args['title'], $args['content'], $enable))
			== FALSE)
		return _error('Unable to insert bug content', 1);
	if(!_sql_query('INSERT INTO daportal_bug (content_id, project_id'
			.', state, type, priority) VALUES'
			." ('$id'"
			.", '".$args['project_id']."'"
			.", 'New'"
			.", '".$args['type']."'"
			.", '".$args['priority']."'"
			.");"))
	{
		_sql_query('DELETE FROM daportal_content'
				." WHERE content_id='$id';");
		return _error('Unable to insert bug', 1);
	}
	$id = _sql_id('daportal_bug', 'bug_id');
	if($enable)
		return project_bug_display(array('id' => $id));
	include('bug_posted.tpl');
}


function project_bug_list($args)
{
	$title = BUG_REPORTS;
	if(isset($args['project_id']))
	{
		if(($args['project'] = _project_name($args['project_id']))
				!= FALSE)
			$project = $args['project'];
	}
	else if(isset($args['project']))
	{
		if(($args['project'] = _sql_single('SELECT name'
				.' FROM daportal_project'
				." WHERE name='".$args['project']."';"))
				!= FALSE)
			$project = $args['project'];
	}
	if(isset($args['user_id']))
	{
		if(($args['username'] = _sql_single('SELECT username'
				.' FROM daportal_user'
				." WHERE user_id='".$args['user_id']."';"))
				!= FALSE)
			$username = $args['username'];
	}
	else if(isset($args['username']))
	{
		if(($args['username'] = _sql_single('SELECT username'
				.' FROM daportal_user'
				." WHERE username='".$args['username']."';"))
				!= FALSE)
			$username = $args['username'];
	}
	if(isset($project))
	{
		if(isset($args['project_id']))
		{
			$project_id = $args['project_id'];
			_project_toolbar($project_id);
		}
		else if(($project_id = _sql_single('SELECT project_id'
				.' FROM daportal_project'
				." WHERE name='$project';")) == FALSE)
			unset($project_id);
		$title.=_FOR_.$project;
	}
	if(isset($username))
		$title.=_BY_.$username;
	print('<h1><img src="modules/project/bug.png" alt=""/> '
			._html_safe($title).'</h1>'."\n");
	$where = '';
	if(strlen($args['project']))
		$where.=" AND daportal_project.name='".$args['project']."'";
	if(strlen($args['username']))
		$where.=" AND daportal_user.username='".$args['username']."'";
	if(strlen($args['state']))
		$where.=" AND daportal_bug.state='".$args['state']."'";
	if(strlen($args['type']))
		$where.=" AND daportal_bug.type='".$args['type']."'";
	if(strlen($args['priority']))
		$where.=" AND daportal_bug.priority='".$args['priority']."'";
	include('bug_list_filter.tpl');
	$order = ' ORDER BY ';
	switch($args['sort'])
	{
		case 'name':	$order.='name DESC'; break;
		case 'project':	$order.='project DESC'; break;
		case 'username':$order.='username DESC'; break;
		case 'state':	$order.='state DESC'; break;
		case 'type':	$order.='type DESC'; break;
		case 'priority':$order.='priority DESC'; break;
		default:
		case 'id':	$order.='bug_id DESC'; break;
	}
	$bugs = _sql_array('SELECT daportal_content.content_id AS content_id'
			.', bug_id AS id, timestamp AS date, title AS name'
			.', content, daportal_project.name AS project, username'
			.', daportal_project.project_id'
			.', state, type, priority'
			.' FROM daportal_content, daportal_bug, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='t'"
			.' AND daportal_content.content_id'
			.'=daportal_bug.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_project.project_id'
			.'=daportal_bug.project_id'
			.$where
			.$order);
	if(!is_array($bugs))
		return _error('Unable to list bugs', 1);
	for($i = 0, $count = count($bugs); $i < $count; $i++)
	{
		$bugs[$i]['icon'] = 'modules/project/bug.png';
		$bugs[$i]['thumbnail'] = 'modules/project/bug.png';
		$bugs[$i]['name'] = _html_safe($bugs[$i]['name']);
		$bugs[$i]['module'] = 'project';
		$bugs[$i]['action'] = 'bug_display';
		$bugs[$i]['nb'] = '<a href="index.php?module=project'
				.'&amp;action=bug_display'
				.'&amp;id='.$bugs[$i]['id'].'">#'
				.$bugs[$i]['id'].'</a>';
		$bugs[$i]['project'] = '<a href="index.php?module=project'
				.'&amp;id='.$bugs[$i]['project_id'].'">'
				._html_safe($bugs[$i]['project'])
				.'</a>';
		$bugs[$i]['date'] = date('d/m/Y H:i',
				strtotime(substr($bugs[$i]['date'], 0, 19)));
	}
	$toolbar = array();
	$link = 'index.php?module=project&action=bug_new'.(isset($project_id)
			? '&project_id='.$project_id : '');
	$toolbar[] = array('icon' => 'modules/project/bug.png',
		'title' => REPORT_A_BUG,
		'link' => $link);
	_module('explorer', 'browse_trusted', array('entries' => $bugs,
			'class' => array('nb' => '#',
					'project' => PROJECT,
					'date' => DATE,
					'state' => STATE,
					'type' => TYPE,
					'priority' => PRIORITY),
			'module' => 'project',
			'action' => 'bug_list',
			'sort' => isset($args['sort']) ? $args['sort'] : 'nb',
			/* FIXME should set args according to filters */
			'view' => 'details',
			'toolbar' => $toolbar));
}


function project_bug_modify($args)
{
	/* FIXME we won't use only bug_update.tpl here but propose a series of
	 * logical actions (choose different project, assign to a user, etc) */
	/* FIXME for instance create a user_assign function with current number
	 * of affected bugs etc */
	/* FIXME maybe also restore original project listing function and
	 * create a project_assign one with current number of (open) bugs etc */
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	$bug = _sql_array('SELECT bug_id AS id, title, content, state, type'
			.', priority'
			.' FROM daportal_bug, daportal_content'
			.' WHERE daportal_bug.content_id'
			.'=daportal_content.content_id'
			." AND bug_id='".$args['id']."';");
	if(!is_array($bug) || count($bug) != 1)
		return _error(INVALID_ARGUMENT);
	$bug = $bug[0];
	$title = 'Modification of bug #'.$bug['id'].': '.$bug['title'];
	include('bug_update.tpl');
}


function project_bug_new($args)
{
	if(!is_numeric($args['project_id'])
			|| !($project = _project_name($args['project_id'])))
		return project_list(array('action' => 'bug_new'));
	$title = REPORT_BUG_FOR.' '.$project;
	$project_id = $args['project_id'];
	include('bug_update.tpl');
}


function project_bug_reply($args)
{
	global $user_id;

	$bug = _sql_array('SELECT daportal_content.content_id AS content_id'
			.', daportal_project.project_id AS project_id'
			.', daportal_user.user_id AS user_id'
			.', daportal_bug.bug_id AS id, timestamp, title'
			.', content, name AS project, username'
			.', state, type, priority'
			.' FROM daportal_content, daportal_bug, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='t'"
			.' AND daportal_content.content_id'
			.'=daportal_bug.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_project.project_id'
			.'=daportal_bug.project_id'
			." AND bug_id='".$args['id']."';");
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug', 1);
	$bug = $bug[0];
	$title = REPLY_TO_BUG.' #'.$bug['id'].': '.$bug['title'];
	require_once('system/user.php');
	$admin = _user_admin($user_id) ? 1 : 0;
	include('bug_display.tpl');
	include('bug_reply_update.tpl');
}


function project_bug_reply_insert($args)
{
	global $user_id;

	/* FIXME */
	require_once('system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], 1))
			== FALSE)
		return _error('Unable to insert bug reply');
	_sql_query('INSERT INTO daportal_bug_reply (content_id, bug_id)'
			.' VALUES '
			." ('$id', '".$args['id']."');");
	/* FIXME insert updates */
	project_bug_display(array('id' => $args['id']));
}


function project_bug_update($args)
{
	global $user_id, $module_id;

	require_once('system/user.php');
	/* FIXME could be the project admin */
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$id = $args['bug_id'];
	if(($content_id = _sql_single('SELECT content_id FROM daportal_bug'
			." WHERE bug_id='".$id."';")) != FALSE)
		_sql_query('UPDATE daportal_content SET'
				." title='".$args['title']."'"
				.", content='".$args['content']."'"
				." WHERE content_id='".$content_id."';");
	_sql_query('UPDATE daportal_bug SET'
			." state='".$args['state']."'"
			.", type='".$args['type']."'"
			.", priority='".$args['priority']."'"
			." WHERE bug_id='$id';");
	project_bug_display(array('id' => $id));
}


function project_config_update($args)
{
	global $user_id, $module_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^project_([a-zA-Z_]+)$', $k, $regs))
			_config_set('project', $regs[1], $args[$k], 0);
	header('Location: index.php?module=project&action=admin');
	exit(0);
}


function project_default($args)
{
	if(isset($args['id']))
		return project_display($args);
	include('default.tpl');
}


function project_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."';")) == FALSE)
		return _error(INVALID_PROJECT);
	//FIXME remove bug reports and comments?
	_sql_query('DELETE FROM daportal_project'
			." WHERE project_id='$id';");
	require_once('system/content.php');
	_content_delete($id);
}


function project_disable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."';")) == FALSE)
		return _error(INVALID_PROJECT);
	require_once('system/content.php');
	_content_disable($id);
	if($args['display'] != 0)
		project_display(array('id' => $id));
}


function project_display($args)
{
	global $user_id;

	require_once('system/user.php');
	$project = _sql_array('SELECT project_id AS id, name, title'
			.', content AS description, daportal_content.enabled'
			.', daportal_content.user_id, username'
			.' FROM daportal_content, daportal_project'
			.', daportal_user'
			." WHERE content_id='".$args['id']."'"
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id;');
	if(!is_array($project) || count($project) != 1)
		return _error('Unable to display project');
	$project = $project[0];
	//FIXME or the user is the administrator of this project
	$admin = _user_admin($user_id);
	$enabled = $project['enabled'] == 't';
	if($enabled == 0 && !$admin)
		return include('project_submitted.tpl');
	$title = $project['name'];
	_project_toolbar($args['id'], $admin);
	include('project_display.tpl');
	$members = array();
	$members[] = array('id' => $project['user_id'],
			'name' => _html_safe($project['username']),
			'icon' => 'modules/user/icon.png',
			'thumbnail' => 'modules/user/icon.png',
			'module' => 'user',
			'action' => 'default',
			'admin' => '<img src="icons/16x16/enabled.png" alt="yes"/>');
	$m = _sql_array('SELECT daportal_user.user_id AS id'
			.', username AS name'
			.' FROM daportal_project_user, daportal_user'
			." WHERE project_id='".$args['id']."'"
			.' AND daportal_project_user.user_id'
			.'=daportal_user.user_id;');
	foreach($m as $n)
		$members[] = array('id' => $n['id'],
				'name' => _html_safe($n['name']),
				'icon' => 'modules/user/icon.png',
				'thumbnail' => 'modules/user/icon.png',
				'module' => 'user', 'action' => 'default',
				'apply_module' => 'project',
				'apply_id' => $n['id'],
				'apply_args' => 'project_id='.$project['id'],
				'admin' => '<img src="icons/16x16/disabled.png" alt="no"/>');
	print('<h2><img src="modules/user/icon.png" alt=""/> '.MEMBERS.'</h2>');
	$explorer = array('view' => 'details', 'entries' => $members,
			'class' => array('admin' => ADMINISTRATOR),
			'module' => 'project', 'action' => 'display',
			'id' => $project['id']);
	$toolbar = array();
	$toolbar[] = array('title' => 'Add member(s)',
			'icon' => 'modules/user/icon.png',
			'link' => 'index.php?module=project&action=member_add'
					.'&id='.$project['id']);
	$toolbar[] = array('title' => 'Delete member(s)',
			'icon' => 'icons/16x16/delete.png',
			'action' => 'member_delete',
			'confirm' => 'delete');
	if($admin)
		$explorer['toolbar'] = $toolbar;
	_module('explorer', 'browse_trusted', $explorer);
}


function project_download($args)
{
	include('download.tpl');
}


function project_enable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."';")) == FALSE)
		return _error(INVALID_PROJECT);
	require_once('system/content.php');
	_content_enable($id);
	if($args['display'] != 0)
		project_display(array('id' => $id));
}


function project_insert($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	if(($id = _content_insert($args['title'], $args['content'])) == FALSE)
		return _error('Unable to insert project content');
	if(!_sql_query('INSERT INTO daportal_project (project_id, name'
			.', cvsroot) VALUES'
			." ('$id', '".$args['name']."', '".$args['cvsroot']."'"
			.');'))
		return _error('Unable to insert project', 1);
	project_display(array('id' => $id));
}


function project_installer($args)
{
	include('installer.tpl');
}


function project_list($args)
{
	global $user_id;

	$title = PROJECT_LIST;
	$where = '';
	if($args['action'] == 'bug_new')
	{
		$title = 'Select project to bug';
		$action = $args['action'];
	}
	else if(isset($args['user_id'])
			&& ($username = _sql_single('SELECT username'
			." FROM daportal_user WHERE user_id='".$args['user_id']
			."';")) != FALSE)
	{
		$title = PROJECTS._BY_.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($title).'</h1>'."\n");
	$projects = _sql_array('SELECT content_id AS id, name, title AS desc'
			.', username AS admin'
			.', daportal_content.user_id AS user_id'
			.' FROM daportal_content, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='1'"
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id'
			.$where
			.' ORDER BY name ASC;');
	if(!is_array($projects))
		return _error('Could not list projects');
	$count = count($projects);
	for($i = 0; $i < $count; $i++)
	{
		$projects[$i]['module'] = 'project';
		$projects[$i]['action'] = 'display';
		if(isset($action))
			$projects[$i]['link'] = 'index.php?module=project'
					.'&action='.$action
					.'&project_id='.$projects[$i]['id'];
		$projects[$i]['icon'] = 'modules/project/icon.png';
		$projects[$i]['thumbnail'] = 'modules/project/icon.png';
		$projects[$i]['admin'] = '<a href="index.php?module=user'
				.'&amp;id='
				._html_safe_link($projects[$i]['user_id'])
				.'">'._html_safe($projects[$i]['admin']).'</a>';
	}
	$toolbar = array();
	require_once('system/user.php');
	$toolbar[] = array('title' => NEW_PROJECT,
			'icon' => 'modules/project/icon.png',
			'link' => 'index.php?module=project&action=new');
	if(_user_admin($user_id))
		return _module('explorer', 'browse_trusted', array(
					'class' => array(
						'admin' => ADMINISTRATOR,
						'desc' => DESCRIPTION),
					'toolbar' => $toolbar,
					'view' => 'details',
					'entries' => $projects));
	_module('explorer', 'browse_trusted', array(
			'class' => array('admin' => ADMINISTRATOR,
					'desc' => DESCRIPTION),
			'view' => 'details',
			'entries' => $projects));
}


function project_member_add($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$project = _sql_array('SELECT project_id AS id, name, user_id'
			.' FROM daportal_project'
			.', daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."';");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			.'Add member to project '.$project['name']."</h1>\n");
	$members = _sql_array('SELECT user_id AS id FROM daportal_project_user'
			." WHERE project_id='".$project['id']."';");
	$where = " WHERE user_id <> '".$project['user_id']."'"
		." AND user_id <> '0'";
	foreach($members as $m)
		$where.=" AND user_id <> '".$m['id']."'";
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.' FROM daportal_user'.$where.';');
	for($i = 0, $cnt = count($users); $i < $cnt; $i++)
	{
		$users[$i]['icon'] = 'modules/user/icon.png';
		$users[$i]['thumbnail'] = 'modules/user/icon.png';
		$users[$i]['module'] = 'user';
		$users[$i]['action'] = 'default';
		$users[$i]['apply_module'] = 'project';
		$users[$i]['apply_id'] = $users[$i]['id'];
		$users[$i]['apply_args'] = 'project_id='.$project['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => 'Add selected users',
			'icon' => 'modules/user/icon.png',
			'action' => 'member_insert',
			'confirm' => 'add');
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $users,
				'module' => 'project',
				'action' => 'display',
				'id' => $project['id']));
}


function project_member_delete($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	_sql_query('DELETE FROM daportal_project_user WHERE '
			." project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."';");
}


function project_member_insert($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	if(($id = _sql_single('SELECT user_id FROM daportal_project_user'
			." WHERE project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."';")) == $args['id'])
		return;
	_sql_query('INSERT INTO daportal_project_user (project_id, user_id)'
			." VALUES ('".$args['project_id']."'"
			.", '".$args['id']."');");
}


function project_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	$project = _sql_array('SELECT project_id AS id, name, title, content'
			.', cvsroot'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."';");
	if(!is_array($project) || count($project) != 1)
		return error('Unable to update project', 1);
	$project = $project[0];
	$title = 'Modification of '.$project['name'];
	include('project_update.tpl');
}


function project_new($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = NEW_PROJECT;
	include('project_update.tpl');
}


function project_package($args)
{
	include('package.tpl');
}


function project_system($args)
{
	global $title, $html;

	$title.=' - '.PROJECTS;
	if($args['action'] == 'browse' && $args['download'] == 1)
		$html = 0;
	else if($args['action'] == 'config_update')
		$html = 0;
}


function project_timeline($args)
{
	require_once('system/content.php');
	if(_content_readable($args['id']) == FALSE)
		return include('project_submitted.tpl');
	$project = _sql_array('SELECT project_id, name, cvsroot'
			.' FROM daportal_project'
			." WHERE project_id='".$args['id']."';");
	if(!is_array($project) || count($project) != 1)
		return _error('Invalid project ID', 1);
	$project = $project[0];
	_project_toolbar($project['project_id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($project['name']).' CVS</h1>'."\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	print('<h1><img src="modules/project/icon.png" alt=""/> '
			._html_safe($project['name'])
			.' '._html_safe(TIMELINE).'</h1>'."\n");
	//FIXME one more hard-coded variable
	if(($fp = @fopen('/Apps/CVS/CVSROOT/history', 'r')) == FALSE)
		return _error('Unable to open history file', 1);
	$entries = array();
	$i = 0;
	$len = strlen('DeforaOS/'.$project['cvsroot']);
	while(!feof($fp))
	{
		$line = fgets($fp);
		$fields = explode('|', $line);
		if(!strlen($fields[4]))
			continue;
		if(strncmp($fields[3], 'DeforaOS/'.$project['cvsroot'], $len)
				!= 0)
			continue;
		_info($line);
		unset($event);
		switch($fields[0][0])
		{
			case 'A': $event = 'Add'; break;
			case 'F': $event = 'Release'; break;
			case 'M': $event = 'Modify'; break;
			case 'R': $event = 'Remove'; break;
		}
		if(!isset($event))
			continue;
		$name = substr($fields[3], $len+1).'/'.$fields[5];
		$date = base_convert(substr($fields[0], 1, 9), 16, 10);
		$date = date('d/m/Y H:i', $date);
		if(($author = _user_id($fields[1])) != 0)
			$author = '<a href="index.php?module=project'
					.'&amp;action=list&amp;user_id='
					.$author.'">'._html_safe($fields[1])
					.'</a>';
		else
			$author = _html_safe($fields[1]);
		$entries[] = array('name' => _html_safe($name),
				'module' => 'project',
				'action' => 'browse',
				'id' => $args['id'],
				'args' => '&file='._html_safe_link($name).',v',
				'date' => _html_safe($date),
				'event' => _html_safe($event),
				'revision' => '<a href="index.php'
						.'?module=project'
						.'&amp;action=browse'
						.'&amp;id='.$args['id']
						.'&amp;file='
						._html_safe_link($name).',v'
						.'&amp;revision='
						._html_safe_link($fields[4])
						.'">'._html_safe($fields[4])
						.'</a>',
				'author' => $author);
	}
	_module('explorer', 'browse_trusted', array(
			'entries' => array_reverse($entries),
			'class' => array('date' => DATE,
					'event' => 'Action',
					'revision' => 'Revision',
					'author' => AUTHOR),
			'view' => 'details'));
	fclose($fp);
}


function project_update($args)
{
	global $user_id;

	require_once('system/content.php');
	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	/* FIXME allow project's admin to update */
	if(!_content_update($args['id'], $args['title'], $args['content']))
		return _error('Could not update project');
	_sql_query('UPDATE daportal_project SET'
			." cvsroot='".$args['cvsroot']."'"
			." WHERE project_id='".$args['id']."';");
	return project_display(array('id' => $args['id']));
}

?>
