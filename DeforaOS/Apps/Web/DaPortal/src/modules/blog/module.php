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


//BlogModule
class BlogModule extends ContentModule
{
	//public
	//methods
	//essential
	//BlogModule::BlogModule
	public function __construct($id, $name)
	{
		parent::__construct($id, $name);
		$this->module_id = $id;
		$this->module_name = _('Blog');
		$this->content_by = _('Post by');
		$this->content_item = _('Blog post');
		$this->content_items = _('Blog posts');
		$this->content_list_title = _('Latest posts');
		$this->content_list_title_by = _('Posts by');
		$this->content_submit = _('New post');
		$this->content_title = _('Planet');
	}
}

?>
