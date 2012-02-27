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
		$this->query_list = $this->project_query_list;
	}


	//protected
	//properties
	//queries
	protected $project_query_list = "SELECT content_id AS id,
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
}

?>
