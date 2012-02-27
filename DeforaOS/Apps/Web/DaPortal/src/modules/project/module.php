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
		}
		return parent::call($engine, $request);
	}


	//protected
	//properties
	//queries
	protected $project_query_list_bugs = "SELECT bug_id AS id,
		bug.content_id AS content_id, bug.timestamp AS date,
	       	daportal_user.user_id AS user_id, username, bug.title AS title,
		bug.enabled AS enabled, state, type, priority,
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
		timestamp AS date, name AS module,
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
		AND daportal_user.enabled='1'";
	protected $project_query_list_projects_user = "SELECT content_id AS id,
		timestamp AS date, name AS module,
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


	//methods
	//ProjectModule::bugList
	protected function bugList($engine, $request)
	{
		$db = $engine->getDatabase();
		$title = _('Bug reports');

		//FIXME really implement
		$query = $this->project_query_list_bugs;
		$query .= ' ORDER BY id DESC';
		if(($res = $db->query($engine, $query)) === FALSE)
			//FIXME return a dialog instead
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Unable to list contents')));
		$page = new Page;
		$page->setProperty('title', $title);
		$page->append('title', array('text' => $title));
		$treeview = $page->append('treeview');
		$treeview->setProperty('columns', array('title' => _('Title'),
			'bug_id' => _('ID'), 'project' => _('Project'),
			'date' => _('Date'), 'state' => _('State'),
			'type' => _('Type'), 'priority' => _('Priority')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('bug_id', '#'.$res[$i]['id']);
			$row->setProperty('id', 'bug_id:'.$res[$i]['id']);
			$row->setProperty('project', $res[$i]['project']);
			$row->setProperty('date', $res[$i]['date']);
			$row->setProperty('state', $res[$i]['state']);
			$row->setProperty('type', $res[$i]['type']);
			$row->setProperty('priority', $res[$i]['priority']);
		}
		return $page;
	}
}

?>
