<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.



require_once('./system/html.php');
require_once('./system/user.php');
require_once('./modules/content/module.php');


//ProjectModule
class ProjectModule extends ContentModule
{
	//public
	//methods
	//essential
	//ProjectModule::ProjectModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('Projects') : $title;
		parent::__construct($id, $name, $title);
		//settings
		$this->content_list_count = 0;
		$this->content_list_order = 'title ASC';
		$this->content_open_stock = 'open';
		//translations
		$this->text_content_admin = _('Projects administration');
		$this->text_content_by = _('Project by');
		$this->text_content_item = _('Project');
		$this->text_content_items = _('Projects');
		$this->text_content_list_title = _('Project list');
		$this->text_content_list_title_by = _('Projects by');
		$this->text_content_more_content = _('More projects...');
		$this->text_content_open = _('Open');
		$this->text_content_submit = _('New project');
		$this->text_content_title = _('Projects');
		//list only projects by default
		$this->query_list = $this->project_query_list_projects;
		$this->query_list_admin
			= $this->project_query_list_admin_projects;
		$this->query_list_user
			= $this->project_query_list_projects_user;
	}


	//ProjectModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		switch(($action = $request->getAction()))
		{
			case 'bug_list':
				return $this->callBugList($engine, $request);
			case 'bug_reply':
				return $this->callBugReply($engine, $request);
			case 'browse':
			case 'bugList':
			case 'bugReply':
			case 'download':
			case 'timeline':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return parent::call($engine, $request, $internal);
	}


	//protected
	//properties
	//queries
	protected $project_query_bug = "SELECT daportal_bug.content_id AS id,
		title, content, timestamp, daportal_user.user_id AS user_id,
		daportal_user.username AS username, bug_id,
		project_id, state, type, priority, assigned
		FROM daportal_content, daportal_bug, daportal_user
		WHERE daportal_content.content_id=daportal_bug.content_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_content.content_id=:content_id";
	protected $project_query_list_admin_projects = "SELECT content_id AS id,
		daportal_content.enabled AS enabled,
		daportal_content.public AS public,
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title
		FROM daportal_content, daportal_module, daportal_user,
		daportal_project
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_project.project_id
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $project_query_list_bugs = "SELECT bug_id,
		bug.content_id AS id, bug.timestamp AS timestamp,
	       	daportal_user.user_id AS user_id, username, bug.title AS title,
		bug.enabled AS enabled, state, type, priority,
		daportal_project.project_id AS project_id,
		project.title AS project
		FROM daportal_content bug, daportal_module, daportal_user,
		daportal_bug, daportal_content project, daportal_project
		WHERE bug.module_id=daportal_module.module_id
		AND bug.user_id=daportal_user.user_id
		AND bug.content_id=daportal_bug.content_id
		AND bug.enabled='1'
		AND bug.public='1'
		AND daportal_bug.project_id=daportal_project.project_id
		AND project.content_id=daportal_project.project_id
		AND project.enabled='1'
		AND project.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $project_query_list_projects = "SELECT content_id AS id,
		daportal_content.enabled AS enabled,
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title, synopsis
		FROM daportal_content, daportal_module, daportal_user,
		daportal_project
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_project.project_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $project_query_list_projects_user = "SELECT content_id AS id,
		daportal_content.enabled AS enabled,
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title, synopsis
		FROM daportal_content, daportal_module, daportal_user,
		daportal_project
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_project.project_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_user.user_id=:user_id";
	protected $project_query_members = "SELECT
		daportal_user.user_id AS user_id, username,
		daportal_project_user.admin AS admin
		FROM daportal_project_user, daportal_user
		WHERE daportal_project_user.user_id=daportal_user.user_id
		AND project_id=:project_id
		AND daportal_user.enabled='1'
		ORDER BY username DESC";
	protected $project_query_project = "SELECT project_id AS id, title,
		daportal_user.user_id AS user_id,
		daportal_user.username AS username,
		content, cvsroot, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_project, daportal_user
		WHERE daportal_content.content_id=daportal_project.project_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND project_id=:content_id";
	protected $project_query_project_by_name = "SELECT project_id AS id,
		title, content, cvsroot, enabled
		FROM daportal_content, daportal_project
		WHERE daportal_content.content_id=daportal_project.project_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_content.title=:title";
	protected $project_query_get = "SELECT daportal_module.name AS module,
		daportal_user.user_id AS user_id,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content, timestamp,
		bug_id, daportal_bug.project_id AS project_id, state, type,
		priority, assigned, cvsroot
		FROM daportal_module, daportal_user, daportal_content
		LEFT JOIN daportal_project
		ON daportal_content.content_id=daportal_project.project_id
		LEFT JOIN daportal_bug
		ON daportal_content.content_id=daportal_bug.content_id
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND (daportal_content.public='1' OR daportal_content.user_id=:user_id)
		AND daportal_content.content_id=:content_id";


	//methods
	//accessors
	//ProjectModule::_get
	protected function _get($engine, $id, $title = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$query = $this->project_query_get;

		if(($res = $db->query($engine, $query, array(
					'module_id' => $this->id,
					'user_id' => $cred->getUserId(),
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		$res = $res[0];
		$res['date'] = $db->formatDate($engine, $res['timestamp']);
		if(!is_numeric($res['bug_id']))
			$res['project_id'] = $res['id'];
		return $res;
	}


	//ProjectModule::getBug
	protected function getBug($engine, $id)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_bug;

		if(($res = $db->query($engine, $query, array(
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		$res = $res[0];
		$res['date'] = $db->formatDate($engine, $res['timestamp']);
		return $res;
	}


	//ProjectModule::getFilter
	protected function getFilter($engine, $request)
	{
		$r = new Request($engine, $this->name, 'bug_list');
		$form = new PageElement('form', array('request' => $r,
				'idempotent' => TRUE));
		$hbox = $form->append('hbox');
		$vbox1 = $hbox->append('vbox');
		$vbox2 = $hbox->append('vbox');
		//FIXME fetch the project name in additional cases
		$vbox1->append('entry', array('name' => 'project',
			'value' => $request->getParameter('project'),
			'text' => _('Project: ')));
		$vbox2->append('entry', array('name' => 'username',
			'value' => $request->getParameter('username'),
			'text' => _('Submitted by: ')));
		//FIXME implement the rest
		$bbox = $vbox2->append('hbox');
		$bbox->append('button', array('stock' => 'reset',
				'type' => 'reset',
				'text' => _('Reset')));
		$bbox->append('button', array('stock' => 'submit',
				'type' => 'submit',
				'text' => _('Filter')));
		return $form;
	}


	//ProjectModule::getProject
	protected function getProject($engine, $id)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_project;

		if(($res = $db->query($engine, $query, array(
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}


	//ProjectModule::_getProjectByName
	protected function _getProjectByName($engine, $name)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_project_by_name;

		if(($res = $db->query($engine, $query, array(
					'title' => $name))) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}


	//ProjectModule::getToolbar
	protected function getToolbar($engine, $content = FALSE)
	{
		$id = (isset($content['id'])) ? $content['id'] : FALSE;

		if(($bug = $this->getBug($engine, $id)) !== FALSE)
			$id = $bug['project_id'];
		if($id === FALSE)
			return FALSE;
		if(($project = $this->getProject($engine, $id)) === FALSE)
			return FALSE;
		$toolbar = new PageElement('toolbar');
		$r = new Request($engine, $this->name, FALSE, $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'home', 'text' => _('Homepage')));
		if(strlen($project['cvsroot']) > 0)
		{
			$r = new Request($engine, $this->name, 'browse', $id,
					$project['title']);
			$toolbar->append('button', array('request' => $r,
					'stock' => 'open',
					'text' => _('Browse')));
			$r = new Request($engine, $this->name, 'timeline', $id,
					$project['title']);
			$toolbar->append('button', array('request' => $r,
					'stock' => 'development',
					'text' => _('Timeline')));
		}
		$r = new Request($engine, $this->name, 'bug_list', $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'bug', 'text' => _('Bug reports')));
		$r = new Request($engine, $this->name, 'download', $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'download',
				'text' => _('Download')));
		if($this->_isManager($engine, $id))
		{
			$r = new Request($engine, $this->name, 'update', $id,
					$project['title']);
			$toolbar->append('button', array('request' => $r,
					'stock' => 'admin',
					'text' => _('Update')));
		}
		return $toolbar;
	}


	//ProjectModule::isManager
	protected function _isManager($engine, $id)
	{
		$cred = $engine->getCredentials();

		$user = new User($engine, $cred->getUserId());
		if($user->isAdmin())
			return TRUE;
		//FIXME implement
		return FALSE;
	}


	//calls
	//ProjectModule::callBrowse
	protected function callBrowse($engine, $request)
	{
		if(($project = $this->getProject($engine, $request->getId(),
				$request->getTitle())) === FALSE)
			return $this->callDefault($engine);
		$title = _('Project: ').$project['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'project',
				'text' => $title));
		$toolbar = $this->getToolbar($engine, $project);
		$page->append($toolbar);
		if(($scm = $this->attachScm($engine)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('An error occurred')));
		$browse = $scm->browse($engine, $project, $request);
		if(is_resource($browse))
			//FIXME set the proper filename
			return $browse;
		$page->append($browse);
		return $page;
	}


	//ProjectModule::callBugList
	protected function callBugList($engine, $request)
	{
		$db = $engine->getDatabase();
		$title = _('Bug reports');
		$error = FALSE;
		$toolbar = FALSE;
		$query = $this->project_query_list_bugs;
		$project = FALSE;

		//XXX unlike ProjectModule::list() here getId() is the project
		//determine the current project
		if(($id = $request->getId()) !== FALSE
				&& ($project = $this->getProject($engine, $id))
				=== FALSE)
			$error = _('Unknown project');
		else if(($name = $request->getParameter('project')) !== FALSE
				&& strlen($name))
		{
			if(($project = $this->_getProjectByName($engine,
					$name)) !== FALSE)
				$id = $project['id'];
			else
				$error = _('Unknown project');
		}
		if($project !== FALSE)
		{
			$title = _('Bug reports for ').$project['title'];
			$toolbar = $this->getToolbar($engine, $project);
			$query .= ' AND daportal_project.project_id=:project_id';
		}
		$filter = $this->getFilter($engine, $request);
		//filter by user_id
		if(($uid = $request->getParameter('user_id')) !== FALSE)
		{
			$title .= _(' by ').$uid; //XXX
			$query .= ' AND bug.user_id=:user_id';
		}
		//sorting out
		switch(($order = $request->getParameter('sort')))
		{
			case 'date':	$order = 'bug.timestamp DESC';	break;
			case 'title':	$order = 'bug.title ASC';	break;
			default:	$order = 'bug_id DESC';		break;
		}
		$query .= ' ORDER BY '.$order;
		//obtain the corresponding bug reports
		if(($res = $db->query($engine, $query, array('user_id' => $uid,
				'project_id' => $id))) === FALSE)
		{
			$error = _('Unable to list bugs');
			//FIXME return a dialog instead
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		}
		//build the page
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		if($toolbar !== FALSE)
			$page->append($toolbar);
		if($error !== FALSE)
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		if($filter !== FALSE)
			$page->append($filter);
		$treeview = $page->append('treeview');
		$treeview->setProperty('columns', array('title' => _('Title'),
			'bug_id' => _('ID'), 'project' => _('Project'),
			'date' => _('Date'), 'state' => _('State'),
			'type' => _('Type'), 'priority' => _('Priority')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $res[$i]['title'],
					'title' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$link = new PageElement('link', array('request' => $r,
					'text' => '#'.$res[$i]['bug_id'],
					'title' => $res[$i]['title']));
			$row->setProperty('bug_id', $link);
			$row->setProperty('id', 'bug_id:'.$res[$i]['id']);
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['project_id'],
					$res[$i]['project']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $res[$i]['project'],
					'title' => $res[$i]['project']));
			$row->setProperty('project', $link);
			$date = $db->formatDate($engine, $res[$i]['timestamp']);
			$row->setProperty('date', $date);
			$row->setProperty('state', $res[$i]['state']);
			$row->setProperty('type', $res[$i]['type']);
			$row->setProperty('priority', $res[$i]['priority']);
		}
		return $page;
	}


	//ProjectModule::callBugReply
	protected function callBugReply($engine, $request)
	{
		$cred = $engine->getCredentials();
		$user = new User($engine, $cred->getUserId());

		if(($bug = $this->getBug($engine, $request->getId(),
				$request->getTitle())) === FALSE)
			return $this->callDefault($engine);
		$project = $this->getProject($engine, $bug['project_id']);
		$title = sprintf(_('Reply to #%u/%s: %s'), $bug['bug_id'],
				$project['title'], $bug['title']);
		$page = new Page(array('title' => $title));
		//title
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//toolbar
		$toolbar = $this->getToolbar($engine, $project);
		$page->append($toolbar);
		//FIXME process the request
		//bug
		$vbox = $page->append('vbox'); //XXX for the title level
		$this->helperPreview($engine, $vbox, $bug);
		//preview
		if($request->getParameter('preview') !== FALSE)
		{
			$title = $request->getParameter('reply_title');
			$content = $request->getParameter('content');
			$reply = array('title' => _('Preview: ').$title,
					'user_id' => $user->getUserId(),
					'username' => $user->getUsername(),
					'date' => $this->timestampToDate(),
					'content' => $content);
			//FIXME really preview a bug reply instead
			$this->helperPreview($engine, $vbox, $reply);
		}
		//form
		$form = $this->formBugReply($engine, $request, $bug, $project);
		$vbox->append($form);
		return $page;
	}


	//ProjectModule::callDefault
	protected function callDefault($engine, $request = FALSE)
	{
		if($request !== FALSE && $request->getId() !== FALSE)
			return $this->callDisplay($engine, $request);
		return $this->callList($engine, $request);
	}


	//ProjectModule::callDownload
	protected function callDownload($engine, $request)
	{
		if(($project = $this->getProject($engine, $request->getId(),
				$request->getTitle())) === FALSE)
			return $this->callDefault($engine);
		$title = _('Project: ').$project['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'project',
				'text' => $title));
		$toolbar = $this->getToolbar($engine, $project);
		$page->append($toolbar);
		//source code
		if(($scm = $this->attachScm($engine)) !== FALSE
				&& ($download = $scm->download($engine,
				$project, $request)) !== FALSE)
			$page->append($download);
		//releases
		$vbox = $page->append('vbox');
		$vbox->append('title', array('text' => _('Releases')));
		$columns = array('filename' => _('Filename'),
			'owner' => _('Owner'), 'group' => _('Group'),
			'date' => _('Date'), 'permissions' => _('Permissions'));
		$view = $vbox->append('treeview', array('columns' => $columns));
		//FIXME really implement
		return $page;
	}


	//ProjectModule::callTimeline
	protected function callTimeline($engine, $request)
	{
		if(($project = $this->getProject($engine, $request->getId(),
				$request->getTitle())) === FALSE)
			return $this->callDefault($engine);
		$title = _('Project: ').$project['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'project',
				'text' => $title));
		$toolbar = $this->getToolbar($engine, $project);
		$page->append($toolbar);
		if(($scm = $this->attachScm($engine)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('An error occurred')));
		$timeline = $scm->timeline($engine, $project, $request);
		$page->append($timeline);
		return $page;
	}


	//forms
	//ProjectModule::formBugReply
	protected function formBugReply($engine, $request, $bug, $project)
	{
		$r = new Request($engine, $this->name, 'bugReply',
				$request->getId(), $request->getTitle());
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		$title = $request->getParameter('reply_title');
		$vbox->append('entry', array('text' => _('Title: '),
				'name' => 'reply_title',
				'value' => $title));
		$vbox->append('textview', array('text' => _('Content: '),
				'name' => 'content',
				'value' => $request->getParameter('content')));
		//FIXME really implement
		$r = new Request($engine, $this->name, FALSE, $bug['id'],
				$bug['title']);
		$box = $vbox->append('buttonbox');
		$box->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		$box->append('button', array('type' => 'submit',
				'stock' => 'preview', 'name' => 'action',
				'value' => 'preview', 'text' => _('Preview')));
		//FIXME add missing buttons
		return $form;
	}


	//ProjectModule::formSubmit
	protected function formSubmit($engine, $request, $content)
	{
		$r = new Request($engine, $this->name, 'submit');
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		$vbox->append('entry', array('name' => 'title',
				'text' => _('Name: '),
				'value' => $request->getTitle()));
		$vbox->append('entry', array('name' => 'synopsis',
				'text' => _('Synopsis: '),
				'value' => $request->getParameter('synopsis')));
		$vbox->append('textview', array('name' => 'content',
				'text' => _('Description: '),
				'value' => $request->getParameter('content')));
		$r = new Request($engine, $this->name);
		$form->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		if($this->canPreview($engine, $request))
			$form->append('button', array('type' => 'submit',
					'stock' => 'preview',
					'name' => 'action',
					'value' => 'preview',
					'text' => _('Preview')));
		$form->append('button', array('type' => 'submit',
				'stock' => 'new', 'name' => 'action',
				'value' => 'submit', 'text' => _('Create')));
		return $form;
	}


	//helpers
	//ProjectModule::helperDisplay
	protected function helperDisplay($engine, $page, $content = FALSE)
	{
		if($content === FALSE)
			return;
		if(isset($content['bug_id']))
			return $this->helperDisplayBug($engine, $page,
					$content);
		return $this->helperDisplayProject($engine, $page, $content);
	}


	//ProjectModule::helperDisplayBug
	protected function helperDisplayBug($engine, $page, $content)
	{
		$project = $this->_get($engine, $content['project_id']);

		$request = new Request($engine, $content['module'], FALSE,
				$content['id'], $content['title']);
		$c = $content;
		$c['title'] = $title = sprintf(_('#%u/%s: %s'),
				$c['bug_id'], $project['title'], $c['title']);
		//title
		$this->helperDisplayTitle($engine, $page, $request, $c);
		//toolbar
		//FIXME pages should render as vbox by default
		$vbox = $page->append('vbox');
		$this->helperDisplayToolbar($engine, $vbox, $request, $content);
		$this->helperDisplayBugMetadata($engine, $vbox, $request,
				$content, $project);
		//content
		$this->helperDisplayText($engine, $vbox, $request, $content);
		//buttons
		$r = new Request($engine, $this->name, 'bugReply',
				$content['id'], $content['title']);
		$vbox->append('button', array('request' => $r,
				'stock' => 'reply', 'text' => _('Reply')));
		$this->helperDisplayButtons($engine, $vbox, $request, $content);
	}


	//ProjectModule::helperDisplayBugMetadata
	protected function helperDisplayBugMetadata($engine, $page, $request,
			$bug, $project)
	{
		$r = new Request($engine, $this->name, FALSE, $project['id'],
				$project['title']);
		$u = new Request($engine, $this->name, 'list', $bug['user_id'],
				$bug['username']);
		$user = is_numeric($bug['assigned'])
			? new User($engine, $bug['assigned']) : FALSE;
		$a = ($user !== FALSE)
			? new Request($engine, $this->name, 'list',
				$user->getUserId(), $user->getUsername())
			: FALSE;

		$page = $page->append('hbox');
		$col1 = $page->append('vbox');
		$col2 = $page->append('vbox');
		$col3 = $page->append('vbox');
		$col4 = $page->append('vbox');
		//project
		$col1->append('label', array('class' => 'bold',
				'text' => _('Project: ')));
		$col2->append('link', array('class' => 'bold', 'request' => $r,
				'text' => $project['title']));
		//submitter
		$col3->append('label', array('class' => 'bold',
				'text' => _('Submitter: ')));
		$col4->append('link', array('request' => $u,
				'text' => $bug['username']));
		//date
		$col1->append('label', array('class' => 'bold',
				'text' => _('Date: ')));
		//XXX should span across columns instead
		$col2->append('label', array('text' => $bug['date']));
		$col3->append('label', array('text' => ' '));
		$col4->append('label', array('text' => ' '));
		//state
		$col1->append('label', array('class' => 'bold',
				'text' => _('State: ')));
		$col2->append('label', array('text' => $bug['state']));
		//type
		$col3->append('label', array('class' => 'bold',
				'text' => _('Type: ')));
		$col4->append('label', array('text' => $bug['type']));
		//priority
		$col1->append('label', array('class' => 'bold',
				'text' => _('Priority: ')));
		$col2->append('label', array('text' => $bug['priority']));
		//assigned
		$col3->append('label', array('class' => 'bold',
				'text' => _('Assigned to: ')));
		if($a !== FALSE)
			$col4->append('link', array('request' => $a,
				'text' => $user->getUsername()));
		else
			$col4->append('label', array('text' => ' '));
	}


	//ProjectModule::helperDisplayDescription
	protected function helperDisplayDescription($engine, $page, $request,
			$content)
	{
		if(!is_array($content) || !isset($content['content'])
				|| strlen($content['content']) == 0)
			return;
		$vbox = $page->append('vbox');
		$vbox->append('title', array('text' => _('Description')));
		$html = HTML::format($engine, $content['content']);
		$vbox->append('htmlview', array('text' => $html));
	}


	//ProjectModule::helperDisplayMembers
	protected function helperDisplayMembers($engine, $page, $request,
			$content)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_members;
		$user = new User($engine, $content['user_id']);

		if(($res = $db->query($engine, $query, array(
				'project_id' => $content['id']))) === FALSE)
			return;
		$vbox = $page->append('vbox');
		$vbox->append('title', array('text' => _('Members')));
		$columns = array('title' => _('Name'),
				'admin' => _('Administrator'));
		$view = $vbox->append('treeview', array('columns' => $columns));
		$no = new PageElement('image', array('stock' => 'no',
			'size' => 16, 'title' => _('Disabled')));
		$yes = new PageElement('image', array('stock' => 'yes',
			'size' => 16, 'title' => _('Enabled')));
		//project owner
		$r = new Request($engine, 'user', FALSE, $user->getUserId(),
				$user->getUsername());
		$link = new PageElement('link', array('request' => $r,
				'stock' => 'user',
				'text' => $user->getUsername()));
		$view->append('row', array('title' => $link, 'admin' => $yes));
		//project members
		foreach($res as $r)
		{
			$row = $view->append('row');
			$r = new Request($engine, 'user', FALSE, $r['user_id'],
					$r['username']);
			$link = new PageElement('link', array('request' => $r,
				'stock' => 'user', 'text' => $r['username']));
			$row->setProperty('title', $link);
			$row->setProperty('admin', $db->isTrue($r['admin'])
					? $yes : $no);
		}
	}


	//ProjectModule::helperDisplayProject
	protected function helperDisplayProject($engine, $page, $content)
	{
		$request = new Request($engine, $content['module'], FALSE,
				$content['id'], $content['title']);
		//title
		$this->helperDisplayTitle($engine, $page, $request, $content);
		//toolbar
		//FIXME pages should render as vbox by default
		$vbox = $page->append('vbox');
		$this->helperDisplayToolbar($engine, $vbox, $request, $content);
		//content
		$this->helperDisplayDescription($engine, $vbox, $request,
				$content);
		//members
		$this->helperDisplayMembers($engine, $vbox, $request, $content);
		//buttons
		$this->helperDisplayButtons($engine, $vbox, $request, $content);
	}


	//ProjectModule::helperDisplayTitle
	protected function helperDisplayTitle($engine, $page, $request,
			$content)
	{
		$title = _('Project: '.$content['title']);

		$page->setProperty('title', $title);
		$title = $page->append('title', array('stock' => $this->name,
			'text' => $title));
	}


	//ProjectModule::helperListButtons
	protected function helperListButtons($engine, $page, $request)
	{
	}


	//ProjectModule::helperListToolbar
	protected function helperListToolbar($engine, $page, $request)
	{
		//XXX assumes $page is the treeview
		if(($columns = $page->getProperty('columns')) !== FALSE)
		{
			$columns['username'] = _('Manager');
			$columns['synopsis'] = _('Description');
			$page->setProperty('columns', $columns);
		}
		return parent::helperListToolbar($engine, $page, $request);
	}


	//useful
	//ProjectModule::attachScm
	protected function attachScm(&$engine)
	{
		global $config; //XXX attach modules per-project instead?

		if(($name = $config->getVariable('module::'.$this->name,
				'scm::backend')) === FALSE)
			$name = 'cvs';
		$filename = './modules/'.$this->name.'/scm/'.$name.'.php';
		$res = include_once($filename);
		if($res === FALSE)
			return FALSE;
		$name = ucfirst($name).'ScmProject';
		$ret = new $name();
		$engine->log('LOG_DEBUG', 'Attaching '.get_class($ret));
		$ret->attach($engine);
		return $ret;
	}
}


//ProjectScm
abstract class ProjectScm
{
	//methods
	//virtual
	abstract public function attach($engine);

	//actions
	abstract public function browse($engine, $project, $request);
	abstract public function download($engine, $project, $request);
	abstract public function timeline($engine, $project, $request);
}

?>
