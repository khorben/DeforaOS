<?php //modules/project/module.php
//FIXME license
//FIXME hide attic option



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['ASSIGNED_TO'] = 'Assigned to';
$text['BROWSE_SOURCE'] = 'Browse source';
$text['BUG_REPORTS'] = 'Bug reports';
$text['CVS_PATH'] = 'CVS path';
$text['FILES'] = 'Files';
$text['INVALID_PROJECT'] = 'Invalid project';
$text['MEMBERS'] = 'Members';
$text['MODIFICATION_OF_BUG_HASH'] = 'Modification of bug #';
$text['MODIFICATION_OF_REPLY_TO_BUG_HASH'] = 'Modification of reply to bug #';
$text['NEW_PROJECT'] = 'New project';
$text['NO_CVS_REPOSITORY'] = 'This project does not have a CVS repository';
$text['PRIORITY'] = 'Priority';
$text['PRIORITY_CHANGED_TO'] = 'Priority changed to';
$text['PROJECT'] = 'Project';
$text['PROJECT_LIST'] = 'Project list';
$text['PROJECT_NAME'] = 'Project name';
$text['PROJECTS'] = 'Projects';
$text['PROJECTS_ADMINISTRATION'] = 'Projects administration';
$text['RELEASES'] = 'Releases';
$text['REPLY_BY'] = 'Reply by';
$text['REPLY_ON'] = 'on';
$text['REPLY_TO_BUG'] = 'Reply to bug';
$text['REPORT_A_BUG'] = 'Report a bug';
$text['REPORT_BUG_FOR'] = 'Report bug for';
$text['REVISION'] = 'Revision';
$text['SCREENSHOTS'] = 'Screenshots';
$text['STATE'] = 'State';
$text['SUBMITTER'] = 'Submitter';
$text['STATE_CHANGED_TO'] = 'State changed to';
$text['TYPE_CHANGED_TO'] = 'Type changed to';
$text['TIMELINE'] = 'Timeline';
global $lang;
if($lang == 'de')
{
	include('./modules/project/lang.de.php');
}
else if($lang == 'fr')
{
	include('./modules/project/lang.fr.php');
}
_lang($text);
define('S_IFDIR', 01000);


function _project_toolbar($id)
{
	global $user_id, $html;

	if(!$html)
		return;
	require_once('./system/user.php');
	$admin = _user_admin($user_id);
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
		$enabled = $project[0]['enabled'] == SQL_TRUE ? 1 : 0;
	}
	include('./modules/project/toolbar.tpl');
}


function _project_name($id)
{
	return _sql_single('SELECT name FROM daportal_project'
			." WHERE project_id='$id';");
}


function project_admin($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(isset($args['id']))
		return project_modify($args);
	print('<h1 class="title project">'._html_safe(PROJECTS_ADMINISTRATION)
			.'</h1>'."\n");
	if(($configs = _config_list('project')))
	{
		print('<h2><img src="modules/project/icon.png" alt=""/>'
				.' Configuration</h2>'."\n");
		$module = 'project';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2><img src="modules/project/icon.png" alt=""/> '
			._html_safe(PROJECT_LIST).'</h2>'."\n");
	$sql = 'SELECT content_id AS id, name, title AS desc, username AS admin'
		.', daportal_content.enabled AS enabled'
		.', daportal_content.user_id AS user_id, cvsroot'
		.' FROM daportal_content, daportal_user, daportal_project'
		.' WHERE daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_content.content_id=daportal_project.project_id'
		.' ORDER BY name ASC';
	$projects = _sql_array($sql);
	if(!is_array($projects))
		return _error('Could not list projects');
	for($i = 0, $cnt = count($projects); $i < $cnt; $i++)
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
		$projects[$i]['enabled'] = ($projects[$i]['enabled']
				== SQL_TRUE) ? 'enabled' : 'disabled';
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
			'action' => 'disable', 'confirm' => 'disable');
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable', 'confirm' => 'enable');
	$toolbar[] = array('title' => DELETE,
			'icon' => 'icons/16x16/delete.png',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $projects,
			'class' => array('enabled' => ENABLED,
					'admin' => ADMINISTRATOR,
					'desc' => DESCRIPTION,
					'cvsroot' => CVS_PATH),
			'toolbar' => $toolbar, 'view' => 'details',
			'module' => 'project', 'action' => 'admin'));
}


function project_browse($args)
{
	$sql = 'SELECT name, cvsroot FROM daportal_content, daportal_project'
		." WHERE project_id='".$args['id']."'"
		.' AND daportal_content.content_id=daportal_project.project_id'
		." AND enabled='1'";
	$project = _sql_array($sql);
	if(!is_array($project) || count($project) != 1
			|| !($cvsrep = _config_get('project', 'cvsroot')))
		return _error(INVALID_PROJECT);
	$cvsrep.='/';
	$project = $project[0];
	_project_toolbar($args['id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1 class="title project">'._html_safe($project['name'])
				.' CVS</h1>'."\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	if(!ereg('^[a-zA-Z0-9. /]+$', $project['cvsroot'])
			|| ereg('\.\.', $project['cvsroot']))
		return _error('Invalid CVSROOT', 1);
	include('./modules/project/browse.php');
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


function project_bug_assign($args)
{
	global $user_id;

	if(!_user_admin($user_id)) //FIXME projects members too
		return _error(PERMISSION_DENIED);
	if(!isset($args['id']) || !isset($args['bug_id']))
		return _error(INVALID_ARGUMENT);
	if(isset($args['user_id']))
	{
		$sql = 'SELECT daportal_user.user_id AS user_id'
			.' FROM daportal_project_user,daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND daportal_user.enabled='1'"
			." AND project_id='".$args['id']."'"
			." AND user_id='".$args['user_id']."'";
		if(_sql_single($sql) != $args['user_id'] && !_user_admin(
				$args['user_id']))
			return _error('Invalid user for assignment');
		if(_sql_query("UPDATE daportal_bug SET assigned='"
					.$args['user_id']."'"
					." WHERE bug_id='".$args['bug_id']."'")
				== FALSE)
			return _error('Could not assign bug');
		/* FIXME should insert a bug_reply with corresponding content */
		return project_bug_display(array('id' => $args['bug_id']));
	}
	/* FIXME should be feasible in one SQL query, and factorize */
	$admin = _sql_array('SELECT daportal_project.project_id AS id, bug_id'
			.', daportal_user.user_id AS user_id, username AS name'
			.' FROM daportal_bug, daportal_project'
			.', daportal_content, daportal_user'
			.' WHERE daportal_bug.project_id'
			.'=daportal_project.project_id'
			.' AND daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND bug_id='".$args['bug_id']."'");
	if(!is_array($admin) || count($admin) != 1)
		return _error('Could not fetch project admin');
	$members = _sql_array('SELECT daportal_project.project_id AS id, bug_id'
			.', daportal_user.user_id AS user_id'
			.', username AS name'
			.' FROM daportal_bug, daportal_project_user'
			.', daportal_user'
			.' WHERE daportal_bug.project_id'
			.'=daportal_project.project_id'
			.' AND daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND daportal_user.enabled='1'"
			." AND bug_id='".$args['bug_id']."'");
	if(!is_array($members))
		return _error('Could not list project members');
	print('<h1 class="title project">Assign bug #'
			._html_safe($args['bug_id']).' to user</h1>'."\n");
	$members = array_merge($admin, $members);
	for($i = 0, $cnt = count($members); $i < $cnt; $i++)
	{
		$members[$i]['module'] = 'project';
		$members[$i]['action'] = 'bug_assign';
		$members[$i]['args'] = '&bug_id='.$members[$i]['bug_id'];
		$members[$i]['args'].='&user_id='.$members[$i]['user_id'];
		$members[$i]['icon'] = 'modules/user/icon.png';
		$members[$i]['thumbnail'] = 'modules/user/icon.png';
	}
	_module('explorer', 'browse', array('entries' => $members));
}


function project_bug_display($args)
{
	global $user_id;

	if(!is_numeric($args['id']))
		return _error(INVALID_ARGUMENT);
	$sql = 'SELECT daportal_content.content_id AS content_id'
		.', daportal_project.project_id AS project_id'
		.', daportal_user.user_id AS user_id, daportal_bug.bug_id AS id'
		.', timestamp, title, content, name AS project, username'
		.', state, type, priority, assigned AS assigned_id'
		.' FROM daportal_content, daportal_bug, daportal_user'
		.', daportal_project WHERE daportal_content.enabled='."'1'"
		.' AND daportal_content.content_id=daportal_bug.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_project.project_id=daportal_bug.project_id'
		." AND bug_id='".$args['id']."'";
	$bug = _sql_array($sql);
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug', 1);
	$bug = $bug[0];
	_project_toolbar($bug['project_id']);
	$title = 'Bug #'.$bug['id'].': '.$bug['title'];
	require_once('./system/user.php');
	$admin = _user_admin($user_id) ? 1 : 0;
	$bug['date'] = strftime(DATE_FORMAT, strtotime(substr($bug['timestamp'],
					0, 19)));
	$bug['assigned'] = is_numeric($bug['assigned_id'])
		? _sql_single('SELECT username FROM daportal_user'
			." WHERE enabled='1'"
			." AND user_id='".$bug['assigned_id']."'") : '';
	include('./modules/project/bug_display.tpl');
	$sql = 'SELECT bug_reply_id AS id, title, content'
		.', daportal_content.content_id AS content_id'
		.', timestamp AS date, state, type, priority'
		.', daportal_user.user_id AS user_id, username'
		.', assigned AS assigned_id'
		.' FROM daportal_bug_reply, daportal_content, daportal_user'
		.' WHERE daportal_bug_reply.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND daportal_content.enabled='1'"
		." AND (daportal_user.enabled='1' OR daportal_user.user_id='0')"
		." AND daportal_bug_reply.bug_id='".$bug['id']."'"
		.' ORDER BY timestamp ASC';
	$replies = _sql_array($sql);
	if(!is_array($replies))
		return _error('Unable to display bug feedback');
	$cnt = count($replies);
	for($i = 0; $i < $cnt; $i++)
	{
		$replies[$i]['date'] = strftime(DATE_FORMAT,
				strtotime(substr($replies[$i]['date'], 0, 19)));
		$reply = $replies[$i];
		$reply['assigned'] = is_numeric($reply['assigned_id'])
			? _sql_single('SELECT username FROM daportal_user'
				." WHERE enabled='1'"
				." AND user_id='".$reply['assigned_id']."'"):'';
		include('./modules/project/bug_reply_display.tpl');
	}
}


function project_bug_insert($args)
{
	global $user_id;

	require_once('./system/content.php');
	require_once('./system/user.php');
	$enable = 0;
	if(_user_admin($user_id)) //FIXME also for project members
		$enable = 1;
	if(($id = _content_insert($args['title'], $args['content'], $enable))
			== FALSE)
		return _error('Unable to insert bug content', 1);
	if(!_sql_query('INSERT INTO daportal_bug (content_id, project_id'
			.', state, type, priority) VALUES'
			." ('$id'".", '".$args['project_id']."'"
			.", 'New'".", '".$args['type']."'"
			.", '".$args['priority']."'".")"))
	{
		_sql_query('DELETE FROM daportal_content'
				." WHERE content_id='$id'");
		return _error('Unable to insert bug', 1);
	}
	$id = _sql_id('daportal_bug', 'bug_id'); //FIXME race condition?
	//send mail
	$to = _sql_array('SELECT username, email'
			.' FROM daportal_project, daportal_content'
			.', daportal_user'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND project_id='".$args['project_id']."';");
	$members = _sql_array('SELECT username, email'
			.' FROM daportal_project_user, daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND project_id='".$args['project_id']."'"
			." AND enabled='1'");
	if(!is_array($to) || !is_array($members))
		_error('Could not list members', 0);
	else
	{
		$to = $to[0]['username'].' <'.$to[0]['email'].'>';
		foreach($members as $m)
			$to.=', '.$m['username'].' <'.$m['email'].'>';
		$title = '[Bug submission] '.$args['title'];
		$content = 'State: New'."\n".'Type: '.$args['type']."\n"
			.'Priority: '.$args['priority']."\n\n"
			.stripslashes($args['content']);
		require_once('./system/mail.php');
		_mail('Administration Team', $to, $title, $content);
	}
	if($enable)
		return project_bug_display(array('id' => $id));
	include('./modules/project/bug_posted.tpl');
}


function project_bug_list($args)
{
	$title = BUG_REPORTS;
	if(isset($args['project_id']))
	{
		if(($args['project'] = _project_name($args['project_id']))
				!= FALSE)
		{
			$project_id = $args['project_id'];
			$project = $args['project'];
		}
	}
	else if(isset($args['project']))
	{
		if(($project_id = _sql_single('SELECT project_id'
				.' FROM daportal_project'
				." WHERE name='".$args['project']."'"))
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
	if(isset($project) && isset($project_id))
	{
		_project_toolbar($project_id);
		$title.=_FOR_.$project;
	}
	if(isset($username))
		$title.=_BY_.$username;
	print('<h1 class="title bug">'._html_safe($title).'</h1>'."\n");
	$where = '';
	if(isset($args['project']) && strlen($args['project']))
		$where.=" AND daportal_project.name='".$args['project']."'";
	if(isset($args['username']) && strlen($args['username']))
		$where.=" AND daportal_user.username='".$args['username']."'";
	if(isset($args['state']) && strlen($args['state']))
		$where.=" AND daportal_bug.state='".$args['state']."'";
	if(isset($args['type']) && strlen($args['type']))
		$where.=" AND daportal_bug.type='".$args['type']."'";
	if(isset($args['priority']) && strlen($args['priority']))
		$where.=" AND daportal_bug.priority='".$args['priority']."'";
	include('./modules/project/bug_list_filter.tpl');
	$order = ' ORDER BY ';
	$sort = isset($args['sort']) ? $args['sort'] : 'nb';
	switch($sort)
	{
		case 'name':	$order.='name DESC';	break;
		case 'project':	$order.='project DESC';	break;
		case 'username':$order.='username DESC';break;
		case 'state':	$order.='state DESC';	break;
		case 'type':	$order.='type DESC';	break;
		case 'priority':$order.='priority DESC';break;
		case 'nb':
		default:	$sort = 'nb';
				$order.='bug_id DESC';	break;
	}
	$sql = 'SELECT daportal_content.content_id AS content_id'
		.', bug_id AS id, timestamp AS date, title, content'
		.', daportal_project.name AS project, username'
		.', daportal_project.project_id AS project_id, state, type'
		.', priority FROM daportal_content, daportal_bug, daportal_user'
		.', daportal_project WHERE daportal_content.enabled='."'1'"
		.' AND daportal_content.content_id=daportal_bug.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND daportal_project.project_id=daportal_bug.project_id';
	$bugs = _sql_array($sql.$where.$order);
	if(!is_array($bugs))
		return _error('Unable to list bugs', 1);
	for($i = 0, $cnt = count($bugs); $i < $cnt; $i++)
	{
		switch($bugs[$i]['state'])
		{
			case 'New': $bugs[$i]['thumbnail']
				 = 'modules/project/bug-new.png'; break;
			case 'Assigned': $bugs[$i]['thumbnail']
				 = 'modules/project/bug-assigned.png'; break;
			case 'Closed': $bugs[$i]['thumbnail']
				 = 'modules/project/bug-closed.png'; break;
			case 'Fixed': case 'Implemented': $bugs[$i]['thumbnail']
				 = 'modules/project/bug-fixed.png'; break;
			default: $bugs[$i]['thumbnail']
				 = 'modules/project/bug.png'; break;
		}
		$bugs[$i]['icon'] = $bugs[$i]['thumbnail'];
		$bugs[$i]['name'] = _html_safe($bugs[$i]['name']);
		$bugs[$i]['module'] = 'project';
		$bugs[$i]['action'] = 'bug_display';
		$bugs[$i]['name'] = $bugs[$i]['title'];
		$bugs[$i]['nb'] = '<a href="index.php?module=project'
				.'&amp;action=bug_display'
				.'&amp;id='.$bugs[$i]['id'].'">#'
				.$bugs[$i]['id'].'</a>';
		$bugs[$i]['project'] = '<a href="index.php?module=project'
				.'&amp;action=bug_list'
				.'&amp;project_id='.$bugs[$i]['project_id'].'">'
				._html_safe($bugs[$i]['project']).'</a>';
		$bugs[$i]['date'] = date('d/m/Y H:i',
				strtotime(substr($bugs[$i]['date'], 0, 19)));
	}
	$toolbar = array();
	$link = 'index.php?module=project&action=bug_new'.(isset($project_id)
			? '&project_id='.$project_id : '');
	$toolbar[] = array('icon' => 'modules/project/bug.png',
		'title' => REPORT_A_BUG, 'link' => $link);
	_module('explorer', 'browse_trusted', array('entries' => $bugs,
			'class' => array('nb' => '#', 'project' => PROJECT,
					'date' => DATE, 'state' => STATE,
					'type' => TYPE, 'priority' => PRIORITY),
			'module' => 'project', 'action' => 'bug_list',
			'sort' => $sort,
			//FIXME should set args according to filters
			'view' => 'details', 'toolbar' => $toolbar));
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

	require_once('./system/user.php');
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
	$title = MODIFICATION_OF_BUG_HASH.$bug['id'].': '.$bug['title'];
	include('./modules/project/bug_update.tpl');
}


function project_bug_new($args)
{
	if(!is_numeric($args['project_id'])
			|| !($project = _project_name($args['project_id'])))
		return project_list(array('action' => 'bug_new'));
	$title = REPORT_BUG_FOR.' '.$project;
	$project_id = $args['project_id'];
	include('./modules/project/bug_update.tpl');
}


function project_bug_reply($reply)
{
	global $user_id, $user_name;

	$bug = _sql_array('SELECT daportal_content.content_id AS content_id'
			.', daportal_project.project_id AS project_id'
			.', daportal_user.user_id AS user_id'
			.', daportal_bug.bug_id AS id, timestamp, title'
			.', content, name AS project, username'
			.', state, type, priority'
			.' FROM daportal_content, daportal_bug, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='1'"
			.' AND daportal_content.content_id'
			.'=daportal_bug.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_project.project_id'
			.'=daportal_bug.project_id'
			." AND bug_id='".$reply['id']."';");
	if(!is_array($bug) || count($bug) != 1)
		return _error('Unable to display bug', 1);
	$bug = $bug[0];
	$bug['date'] = strftime(DATE_FORMAT, strtotime(substr($bug['timestamp'],
					0, 19)));
	$title = REPLY_TO_BUG.' #'.$bug['id'].': '.$bug['title'];
	require_once('./system/user.php');
	$admin = _user_admin($user_id) ? 1 : 0;
	if(isset($reply['preview']))
	{
		include('./modules/project/bug_display.tpl');
		unset($reply['id']); //XXX
		$reply['title'] = stripslashes($reply['title']);
		$reply['date'] = strftime(DATE_FORMAT);
		$reply['user_id'] = $user_id;
		$reply['username'] = $user_name;
		//FIXME check state, type and priority are coherent
		if(strlen($reply['state']))
			$reply['state'] = stripslashes($reply['state']);
		else
			unset($reply['state']);
		if(strlen($reply['type']))
			$reply['type'] = stripslashes($reply['type']);
		else
			unset($reply['type']);
		if(strlen($reply['priority']))
			$reply['priority'] = stripslashes($reply['priority']);
		else
			unset($reply['priority']);
		$reply['content'] = stripslashes($reply['content']);
		include('./modules/project/bug_reply_display.tpl');
		return include('./modules/project/bug_reply_update.tpl');
	}
	if(!isset($reply['submit']))
	{
		include('./modules/project/bug_display.tpl');
		unset($reply['id']); //XXX
		$reply['title'] = 'Re: '.$bug['title'];
		return include('./modules/project/bug_reply_update.tpl');
	}
	project_bug_reply_insert($reply);
}


function project_bug_reply_insert($args)
{
	global $user_id;

	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], 1))
			== FALSE)
		return _error('Unable to insert bug reply');
	$fields = '';
	$values = '';
	$update = '';
	require_once('./system/user.php');
	if(_user_admin($user_id)) //FIXME
	{
		if(strlen($args['state']))
		{
			$fields.=', state';
			$values.=", '".$args['state']."'";
			$update.=", state='".$args['state']."'";
		}
		if(strlen($args['type']))
		{
			$fields.=', type';
			$values.=", '".$args['type']."'";
			$update.=", type='".$args['type']."'";
		}
		if(strlen($args['priority']))
		{
			$fields.=', priority';
			$values.=", '".$args['priority']."'";
			$update.=", priority='".$args['priority']."'";
		}
	}
	_sql_query('INSERT INTO daportal_bug_reply'
			.' (content_id, bug_id'.$fields.') VALUES '
			." ('$id', '".$args['id']."'".$values.');');
	if(strlen($update))
		_sql_query('UPDATE daportal_bug SET'
				." bug_id='".$args['id']."'".$update
				." WHERE bug_id='".$args['id']."';");
	project_bug_display(array('id' => $args['id']));
	//send mail
	if(($project_id = _sql_single('SELECT project_id FROM daportal_bug'
			." WHERE bug_id='".$args['id']."';")) == FALSE)
		return _error('Could not determine project', 0);
	$ba = _sql_array('SELECT username, email' //bug author
			.' FROM daportal_bug, daportal_content, daportal_user'
			.' WHERE daportal_bug.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id'
			.'=daportal_user.user_id'
			." AND bug_id='".$args['id']."';");
	$pa = _sql_array('SELECT username, email' //project admin
			.' FROM daportal_project, daportal_content'
			.', daportal_user'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND project_id='$project_id';");
	$assigned = _sql_array('SELECT username, email' //assigned member
			.' FROM daportal_bug, daportal_user'
			.' WHERE daportal_bug.assigned=daportal_user.user_id'
			." AND bug_id='".$args['id']."';");
	$members = _sql_array('SELECT username, email' //all members
			.' FROM daportal_project_user, daportal_user'
			.' WHERE daportal_project_user.user_id'
			.'=daportal_user.user_id'
			." AND project_id='$project_id' AND enabled='1';");
	if(!is_array($ba) || !is_array($pa) || !is_array($assigned)
			|| !is_array($members))
		return _error('Could not list addresses for mailing');
	$array = count($assigned) == 1 ? array_merge($ba, $pa, $assigned)
		: array_merge($ba, $pa, $members);
	if(count($array) == 0)
		return _error('No recipients for mailing');
	$rcpt = array();
	foreach($array as $a)
		$rcpt[$a['username']] = $a['email'];
	$keys = array_keys($rcpt);
	$to = $keys[0].' <'.$rcpt[$keys[0]].'>';
	$cnt = count($keys);
	for($i = 1; $i < $cnt; $i++)
		$to.=', '.$keys[$i].' <'.$rcpt[$keys[$i]].'>';
	$title = '[Bug reply] '.$args['title'];
	$content = 'State: '.$args['state']."\n".'Type: '.$args['type']."\n"
		.'Priority: '.$args['priority']."\n\n"
		.stripslashes($args['content']);
	require_once('./system/mail.php');
	_mail('Administration Team', $to, $title, $content);
}


function project_bug_reply_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$admin = 1;
	$reply = _sql_array('SELECT bug_reply_id AS id, bug_id, title, content'
			.', timestamp AS date'
			.', state, type, priority, daportal_user.user_id'
			.', username, assigned AS assigned_id'
			.' FROM daportal_bug_reply, daportal_content'
			.', daportal_user'
			.' WHERE daportal_bug_reply.content_id'
			.'=daportal_content.content_id'
			.' AND daportal_content.user_id=daportal_user.user_id'
			." AND daportal_content.enabled='1'"
			." AND (daportal_user.enabled='1'"
			." OR daportal_user.user_id='0')"
			." AND bug_reply_id='".$args['id']."';");
	if(!is_array($reply) || count($reply) != 1)
		return _error('Unable to modify bug feedback');
	$reply = $reply[0];
	print('<h1 class="title bug">'
			._html_safe(MODIFICATION_OF_REPLY_TO_BUG_HASH
				.$reply['bug_id'].': '.$reply['title'])
			.'</h1>'."\n");
	$reply['assigned'] = _sql_single('SELECT username'
			.' FROM daportal_user'
			." WHERE enabled='1'"
			." AND user_id='".$reply['assigned_id']."';");
	include('./modules/project/bug_reply_update.tpl');
}


function project_bug_reply_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(!($id = _sql_single('SELECT content_id FROM daportal_bug_reply'
			." WHERE bug_reply_id='".$args['id']."'")))
		return _error(INVALID_ARGUMENT);
	if(!($bug_id = _sql_single('SELECT bug_id FROM daportal_bug_reply'
			." WHERE bug_reply_id='".$args['id']."'")))
		return _error(INVALID_ARGUMENT);
	_sql_query('UPDATE daportal_content SET'
			." title='".$args['title']."'"
			.", content='".$args['content']."'"
			." WHERE content_id='$id';");
	$sql = ' state='.(strlen($args['state']) ? "'".$args['state']."'"
			: 'NULL');
	$sql.=', type='.(strlen($args['type']) ? "'".$args['type']."'"
			: 'NULL');
	$sql.=', priority='.(strlen($args['priority'])
			? "'".$args['priority']."'" : 'NULL');
	_sql_query('UPDATE daportal_bug_reply SET'.$sql
			." WHERE bug_reply_id='".$args['id']."'");
	project_bug_display(array('id' => $bug_id));
}


function project_bug_update($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	//FIXME could be the project admin
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$id = $args['bug_id'];
	if(($content_id = _sql_single('SELECT content_id FROM daportal_bug'
			." WHERE bug_id='".$id."'")) != FALSE)
		_sql_query('UPDATE daportal_content SET'
				." title='".$args['title']."'"
				.", content='".$args['content']."'"
				." WHERE content_id='".$content_id."'");
	_sql_query('UPDATE daportal_bug SET'
			." state='".$args['state']."'"
			.", type='".$args['type']."'"
			.", priority='".$args['priority']."'"
			." WHERE bug_id='$id'");
	project_bug_display(array('id' => $id));
}


function project_config_update($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
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
	include('./modules/project/default.tpl');
}


function project_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."';")) == FALSE)
		return _error(INVALID_PROJECT);
	//FIXME remove bug reports and replies?
	_sql_query('DELETE FROM daportal_project'
			." WHERE project_id='$id';");
	require_once('./system/content.php');
	_content_delete($id);
}


function project_disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."';")) == FALSE)
		return _error(INVALID_PROJECT);
	require_once('./system/content.php');
	_content_disable($id);
	if($args['display'] != 0)
		project_display(array('id' => $id));
}


function project_display($args)
{
	global $user_id;

	require_once('./system/user.php');
	$project = _sql_array('SELECT project_id AS id, name, title'
			.', content AS description'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'
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
	$enabled = $project['enabled'] == SQL_TRUE;
	if($enabled == 0 && !$admin)
	{
		return include('./modules/project/project_submitted.tpl');
	}
	$title = $project['name'];
	_project_toolbar($args['id']);
	include('./modules/project/project_display.tpl');
	$members = array();
	$members[] = array('id' => $project['user_id'],
			'name' => _html_safe($project['username']),
			'title' => _html_safe($project['username']),
			'icon' => 'modules/user/icon.png',
			'thumbnail' => 'modules/user/icon.png',
			'module' => 'user', 'action' => 'default',
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
				'title' => _html_safe($n['name']),
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
			'action' => 'member_delete', 'confirm' => DELETE);
	if($admin)
		$explorer['toolbar'] = $toolbar;
	_module('explorer', 'browse_trusted', $explorer);
}


function project_download($args)
{
	if(!isset($args['id']) || !is_numeric($args['id']))
	{
		return include('./modules/project/download.tpl');
	}
	$category_id = _module_id('category');
	$download_id = _module_id('download');
	if($category_id == 0 || $download_id == 0)
		return _error('Both category and download modules must be'
				.' installed');
	$project = _sql_array('SELECT name, title'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND daportal_content.enabled='1'"
			." AND project_id='".$args['id']."';");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	_project_toolbar($args['id']);
	print('<h1 class="title project">'._html_safe($project['name']).': '
		._html_safe(FILES).'</h1>'."\n");
	require_once('./system/mime.php');
	/* FIXME factorize code */
	$sql = 'SELECT daportal_content.content_id AS id, mode'
		.', daportal_content.title AS name'
		.', daportal_content.user_id AS user_id, username'
		.' FROM daportal_download, daportal_content, daportal_user'
		.', daportal_category_content cc2, daportal_content dc2'
		.', daportal_category_content cc3, daportal_content dc3'
		.' WHERE daportal_download.content_id'
		.'=daportal_content.content_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		.' AND cc2.content_id=daportal_content.content_id'
		.' AND cc3.content_id=daportal_content.content_id'
		.' AND cc2.category_id=dc2.content_id'
		.' AND cc3.category_id=dc3.content_id'
		." AND dc2.title='".$project['name']."'"
		." AND daportal_content.enabled='1'"
		." AND daportal_user.enabled='1' AND daportal_user.admin='1'"
		." AND dc2.enabled='1' AND dc3.enabled='1'";
	/* screenshots */
	/* FIXME project members should be trusted too */
	$files = _sql_array($sql." AND dc3.title='screenshot'");
	if(is_array($files) && ($cnt = count($files)) > 0)
	{
		print('<h2>'._html_safe(SCREENSHOTS).'</h2>'."\n");
		for($i = 0; $i < $cnt; $i++)
		{
			$files[$i]['module'] = 'download';
			$files[$i]['action'] = 'download';
			$mime = _mime_from_ext($files[$i]['name']);
			if(strncmp($mime, 'image/', 6) == 0)
			{
				$files[$i]['icon'] = 'icons/16x16/mime/'
					.(is_readable('icons/16x16/mime/'
								.$mime.'.png'))
						? $mime.'.png' : 'default.png';
				$files[$i]['thumbnail'] = 'index.php'
					.'?module=download&action=download&id='.$files[$i]['id'];
			}
			else if($files[$i]['mode'] & S_IFDIR)
			{
				$files[$i]['icon'] = 'icons/16x16'
					.'/mime/folder.png';
				$files[$i]['thumbnail'] = 'icons/48x48'
					.'/mime/folder.png';
			}
			else
			{
				if(is_readable('icons/48x48/mime/'.$mime.'.png'))
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/'.$mime.'.png';
				else
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/default.png';
				$files[$i]['icon'] = $files[$i]['thumbnail'];
				if(is_readable('icons/16x16/mime/'.$mime
							.'.png'))
					$files[$i]['icon'] = 'icons'
						.'/16x16/mime/'.$mime.'.png';
			}
		}
		_module('explorer', 'browse', array('entries' => $files,
					'view' => 'thumbnails',
					'toolbar' => 0));
	}
	/* releases */
	/* FIXME project members should be trusted too */
	$files = _sql_array($sql." AND dc3.title='release'");
	if(is_array($files) && ($cnt = count($files)) > 0)
	{
		print('<h2>'._html_safe(RELEASES).'</h2>'."\n");
		for($i = 0; $i < $cnt; $i++)
		{
			$files[$i]['module'] = 'download';
			$files[$i]['action'] = 'default';
			$mime = _mime_from_ext($files[$i]['name']);
			if($files[$i]['mode'] & S_IFDIR)
			{
				$files[$i]['icon'] = 'icons/16x16'
					.'/mime/folder.png';
				$files[$i]['thumbnail'] = 'icons/48x48'
					.'/mime/folder.png';
			}
			else
			{
				if(is_readable('icons/48x48/mime/'.$mime.'.png'))
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/'.$mime.'.png';
				else
					$files[$i]['thumbnail'] = 'icons/48x48'
						.'/mime/default.png';
				$files[$i]['icon'] = $files[$i]['thumbnail'];
				if(is_readable('icons/16x16/mime/'.$mime
							.'.png'))
					$files[$i]['icon'] = 'icons'
						.'/16x16/mime/'.$mime.'.png';
			}
		}
		_module('explorer', 'browse', array('entries' => $files));
	}
}


function project_enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(($id = _sql_single('SELECT project_id FROM daportal_project'
			." WHERE project_id='".$args['id']."'")) == FALSE)
		return _error(INVALID_PROJECT);
	require_once('./system/content.php');
	_content_enable($id);
	if($args['display'] != 0)
		project_display(array('id' => $id));
}


function project_insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], $args['content'])) == FALSE)
		return _error('Unable to insert project content');
	if(!_sql_query('INSERT INTO daportal_project (project_id, name'
			.', cvsroot) VALUES'
			." ('$id', '".$args['name']."', '".$args['cvsroot']."'"
			.')'))
		return _error('Unable to insert project', 1);
	project_display(array('id' => $id));
}


function project_installer($args)
{
	include('./modules/project/installer.tpl');
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
			."'")) != FALSE)
	{
		$title = PROJECTS._BY_.$username;
		$where = " AND daportal_content.user_id='".$args['user_id']."'";
	}
	print('<h1 class="title project">'._html_safe($title).'</h1>'."\n");
	$projects = _sql_array('SELECT content_id AS id, name, title'
			.', username AS admin'
			.', daportal_content.user_id AS user_id'
			.' FROM daportal_content, daportal_user'
			.', daportal_project'
			." WHERE daportal_content.enabled='1'"
			.' AND daportal_content.user_id=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_project.project_id'
			.$where.' ORDER BY name ASC');
	if(!is_array($projects))
		return _error('Could not list projects');
	for($i = 0, $cnt = count($projects); $i < $cnt; $i++)
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
			.'&amp;id='._html_safe_link($projects[$i]['user_id'])
			.'">'._html_safe($projects[$i]['admin']).'</a>';
		$projects[$i]['desc'] = $projects[$i]['title'];
	}
	$args = array('entries' => $projects, 'view' => 'details',
			'class' => array('admin' => ADMINISTRATOR,
			'desc' => DESCRIPTION));
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$args['toolbar'] = array(array('title' => NEW_PROJECT,
				'link' => 'index.php?module=project&action=new',
				'icon' => 'modules/project/icon.png'));
	return _module('explorer', 'browse_trusted', $args);
}


function project_member_add($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$project = _sql_array('SELECT project_id AS id, name, user_id'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error(INVALID_PROJECT);
	$project = $project[0];
	print('<h1 class="title project">Add member to project '
			.$project['name']."</h1>\n");
	$members = _sql_array('SELECT user_id AS id FROM daportal_project_user'
			." WHERE project_id='".$project['id']."'");
	$where = " WHERE user_id <> '".$project['user_id']."'"
		." AND user_id <> '0'";
	foreach($members as $m)
		$where.=" AND user_id <> '".$m['id']."'";
	$users = _sql_array('SELECT user_id AS id, username AS name'
			.' FROM daportal_user'.$where);
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
			'action' => 'member_insert', 'confirm' => 'add');
	_module('explorer', 'browse', array('toolbar' => $toolbar,
				'entries' => $users, 'module' => 'project',
				'action' => 'display', 'id' => $project['id']));
}


function project_member_delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	_sql_query('DELETE FROM daportal_project_user WHERE '
			." project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."'");
}


function project_member_insert($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	if(($id = _sql_single('SELECT user_id FROM daportal_project_user'
			." WHERE project_id='".$args['project_id']."'"
			." AND user_id='".$args['id']."'")) == $args['id'])
		return;
	_sql_query('INSERT INTO daportal_project_user (project_id, user_id)'
			." VALUES ('".$args['project_id']."'"
			.", '".$args['id']."')");
}


function project_modify($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED, 1);
	$project = _sql_array('SELECT project_id AS id, name, title, content'
			.', cvsroot'
			.' FROM daportal_project, daportal_content'
			.' WHERE daportal_project.project_id'
			.'=daportal_content.content_id'
			." AND project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return error('Unable to update project', 1);
	$project = $project[0];
	_project_toolbar($project['id']);
	$title = 'Modification of '.$project['name'];
	include('./modules/project/project_update.tpl');
}


function project_new($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$title = NEW_PROJECT;
	include('./modules/project/project_update.tpl');
}


function project_package($args)
{
	include('./modules/project/package.tpl');
}


function project_system($args)
{
	global $title, $html;

	$title.=' - '.PROJECTS;
	if(!isset($args['action']))
		return;
	if($args['action'] == 'browse' && $args['download'] == 1)
		$html = 0;
	else if($args['action'] == 'config_update')
		$html = 0;
}


function project_timeline($args)
{
	require_once('./system/content.php');
	if(_content_readable($args['id']) == FALSE)
	{
		return include('./modules/project/project_submitted.tpl');
	}
	$project = _sql_array('SELECT project_id, name, cvsroot'
			.' FROM daportal_project'
			." WHERE project_id='".$args['id']."'");
	if(!is_array($project) || count($project) != 1)
		return _error('Invalid project ID', 1);
	$project = $project[0];
	_project_toolbar($project['project_id']);
	if(strlen($project['cvsroot']) == 0)
	{
		print('<h1 class="title project">'._html_safe($project['name'])
				.' CVS</h1>'."\n");
		return _info(NO_CVS_REPOSITORY, 1);
	}
	print('<h1 class="title project">'._html_safe($project['name'])
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
		$icon = '';
		switch($fields[0][0])
		{
			case 'A': $event = 'Add'; $icon = 'added'; break;
			case 'F': $event = 'Release'; break;
			case 'M': $event = 'Modify'; $icon = 'modified'; break;
			case 'R': $event = 'Remove'; $icon = 'removed'; break;
		}
		if(!isset($event))
			continue;
		if(strlen($icon))
			$icon = 'modules/project/cvs-'.$icon.'.png';
		$name = substr($fields[3], $len+1).'/'.$fields[5];
		$date = base_convert(substr($fields[0], 1, 9), 16, 10);
		$date = date('d/m/Y H:i', $date);
		if(($author = _user_id($fields[1])) != 0)
			$author = '<a href="index.php?module=project'
				.'&amp;action=list&amp;user_id='
				.$author.'">'._html_safe($fields[1]).'</a>';
		else
			$author = _html_safe($fields[1]);
		$entries[] = array('module' => 'project', 'action' => 'browse',
				'id' => $args['id'],
				'args' => '&file='._html_safe_link($name).',v',
				'name' => _html_safe($name),
				'icon' => $icon, 'thumbnail' => $icon,
				'date' => _html_safe($date),
				'event' => _html_safe($event),
				'revision' => '<a href="index.php'
					.'?module=project&amp;action=browse'
					.'&amp;id='.$args['id']
					.'&amp;file='._html_safe_link($name)
					.',v&amp;revision='
					._html_safe_link($fields[4])
					.'">'._html_safe($fields[4]).'</a>',
				'author' => $author);
	}
	$toolbar = array();
	$toolbar[] = array('title' => BACK, 'icon' => 'icons/16x16/back.png',
			'link' => 'javascript:history.back()');
	$toolbar[] = array('title' => FORWARD,
			'icon' => 'icons/16x16/forward.png',
			'link' => 'javascript:history.forward()');
	$toolbar[] = array();
	$toolbar[] = array('title' => REFRESH,
			'icon' => 'icons/16x16/refresh.png',
			'link' => 'javascript:location.reload()');
	_module('explorer', 'browse_trusted', array(
			'entries' => array_reverse($entries),
			'class' => array('date' => DATE,
					'event' => 'Action',
					'revision' => REVISION,
					'author' => AUTHOR),
			'toolbar' => $toolbar, 'view' => 'details'));
	fclose($fp);
}


function project_update($args)
{
	global $user_id;

	require_once('./system/content.php');
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	//FIXME allow project's admin to update
	if(!_content_update($args['id'], $args['title'], $args['content']))
		return _error('Could not update project');
	_sql_query('UPDATE daportal_project SET'
			." cvsroot='".$args['cvsroot']."'"
			." WHERE project_id='".$args['id']."'");
	return project_display(array('id' => $args['id']));
}

?>
