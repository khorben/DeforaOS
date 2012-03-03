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



require_once('./modules/content/module.php');
require_once('./system/user.php');


//ProjectModule
class ProjectModule extends ContentModule
{
	//public
	//methods
	//essential
	//ProjectModule::ProjectModule
	public function __construct($id, $name)
	{
		parent::__construct($id, $name);
		$this->module_id = $id;
		$this->module_name = _('Projects');
		$this->module_content = _('Project');
		$this->module_contents = _('Projects');
		$this->content_list_count = 0;
		$this->content_list_order = 'title ASC';
		$this->content_list_title = _('Project list');
		$this->content_open_text = _('Open');
		//list only projects by default
		$this->query_list = $this->project_query_list_projects;
		$this->query_list_user
			= $this->project_query_list_projects_user;
	}


	//ProjectModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'bug_list':
				return $this->bugList($engine, $request);
			case 'download':
			case 'timeline':
				/* FIXME really implement */
				return $this->display($engine, $request);
		}
		return parent::call($engine, $request);
	}


	//protected
	//properties
	//queries
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
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title
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
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title,
	       	daportal_content.enabled AS enabled
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
	protected $project_query_bug = "SELECT title, content,
		project_id
		FROM daportal_content, daportal_bug
		WHERE daportal_content.content_id=daportal_bug.content_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_content.content_id=:content_id";
	protected $project_query_project = "SELECT project_id AS id, title,
		content, cvsroot, enabled
		FROM daportal_content, daportal_project
		WHERE daportal_content.content_id=daportal_project.project_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND project_id=:content_id";
	protected $project_query_get = "SELECT daportal_module.name AS module,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content, timestamp,
		bug_id, daportal_bug.project_id AS project_id, priority, cvsroot
		FROM daportal_module, daportal_user, daportal_content
		LEFT JOIN daportal_project
		ON daportal_content.content_id=daportal_project.project_id
		LEFT JOIN daportal_bug
		ON daportal_content.content_id=daportal_bug.content_id
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_content.public='1'
		AND daportal_content.content_id=:content_id";


	//methods
	//accessors
	//ProjectModule::_get
	protected function _get($engine, $id, $title = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_get;

		if(($res = $db->query($engine, $query, array(
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		$res = $res[0];
		$res['date'] = substr($res['timestamp'], 0, 19);
		$res['date'] = strtotime($res['date']);
		$res['date'] = date(_('d/m/Y H:i'), $res['date']);
		if(!is_numeric($res['bug_id']))
			$res['project_id'] = $res['id'];
		return $res;
	}


	//ProjectModule::_getBug
	protected function _getBug($engine, $id)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_bug;

		if(($res = $db->query($engine, $query, array(
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}


	//ProjectModule::_getFilter
	protected function _getFilter($engine, $request)
	{
		$r = new Request($engine, $this->name, 'bug_list');
		$form = new PageElement('form', array('request' => $r,
				'idempotent' => TRUE));
		$hbox = $form->append('hbox');
		$vbox1 = $hbox->append('vbox');
		$vbox2 = $hbox->append('vbox');
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


	//ProjectModule::_getProject
	protected function _getProject($engine, $id)
	{
		$db = $engine->getDatabase();
		$query = $this->project_query_project;

		if(($res = $db->query($engine, $query, array(
					'content_id' => $id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}


	//ProjectModule::_getToolbar
	protected function _getToolbar($engine, $id)
	{
		if(($bug = $this->_getBug($engine, $id)) !== FALSE)
			$id = $bug['project_id'];
		if($id === FALSE)
			return FALSE;
		if(($project = $this->_getProject($engine, $id)) === FALSE)
			return FALSE;
		$toolbar = new PageElement('toolbar');
		$r = new Request($engine, $this->name, FALSE, $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'home', 'text' => _('Homepage')));
		$r = new Request($engine, $this->name, 'browse', $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'open', 'text' => _('Browse')));
		$r = new Request($engine, $this->name, 'timeline', $id,
				$project['title']);
		$toolbar->append('button', array('request' => $r,
				'stock' => 'development',
				'text' => _('Timeline')));
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


	//useful
	//ProjectModule::bugList
	protected function bugList($engine, $request)
	{
		$db = $engine->getDatabase();
		$title = _('Bug reports');
		$error = _('Unable to list bugs');
		$toolbar = FALSE;
		$query = $this->project_query_list_bugs;

		//XXX unlike ProjectModule::list() here getid() is the project
		//determine the current project
		if(($id = $request->getId()) !== FALSE && is_numeric($id)
				&& ($project = $this->_getProject($engine, $id))
				!== FALSE)
		{
			$title = _('Bug reports for ').$project['title'];
			$toolbar = $this->_getToolbar($engine, $id);
			$query .= ' AND daportal_project.project_id=:project_id';
		}
		$filter = $this->_getFilter($engine, $request);
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
			//FIXME return a dialog instead
			return new PageElement('dialog', array(
					'type' => 'error', 'error' => $error));
		//build the page
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title));
		if($toolbar !== FALSE)
			$page->appendElement($toolbar);
		if($filter !== FALSE)
			$page->appendElement($filter);
		$treeview = $page->append('treeview');
		$treeview->setProperty('columns', array('title' => _('Title'),
			'bug_id' => _('ID'), 'project' => _('Project'),
			'date' => _('Date'), 'state' => _('State'),
			'type' => _('Type'), 'priority' => _('Priority')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('title', $res[$i]['title']);
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
				'text' => '#'.$res[$i]['bug_id']));
			$row->setProperty('bug_id', $link);
			$row->setProperty('id', 'bug_id:'.$res[$i]['id']);
			$row->setProperty('project', $res[$i]['project']);
			$date = $this->_timestampToDate($res[$i]['timestamp']);
			$row->setProperty('date', $date);
			$row->setProperty('state', $res[$i]['state']);
			$row->setProperty('type', $res[$i]['type']);
			$row->setProperty('priority', $res[$i]['priority']);
		}
		return $page;
	}


	//ProjectModule::_display
	protected function _display($engine, $id, $title = FALSE)
	{
		if($id === FALSE)
			return $this->default();
		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return FALSE;
		//for projects
		$title = _('Project: ').$content['title'];
		$r = new Request($engine, $this->name, 'list');
		$link = new PageElement('link', array('request' => $r,
				'stock' => 'back',
				'text' => _('More projects...')));
		if(is_numeric($content['bug_id']))
		{
			//for bug reports
			//XXX may fail
			$project = $this->_getProject($engine,
					$content['project_id']);
			$title = sprintf(_("Bug report #%u/%s: %s"),
					$content['bug_id'], $project['title'],
					$content['title']);
			$r = new Request($engine, $this->name, 'bug_list',
					$project['id'], $project['title']);
			$link = new PageElement('link', array('request' => $r,
					'stock' => 'back',
					'text' => _('Other bug reports...')));
		}
		else
			$project = $content;
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
			'text' => $title));
		if(($toolbar = $this->_getToolbar($engine, $project['id']))
				!== FALSE)
			$page->appendElement($toolbar);
		$page->append('label', array('text' => $content['content']."\n"));
		$page->appendElement($link);
		return $page;
	}
}

?>
