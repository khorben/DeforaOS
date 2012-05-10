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


//ArticleModule
class ArticleModule extends ContentModule
{
	//public
	//methods
	//essential
	//ArticleModule::ArticleModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('Articles') : $title;
		parent::__construct($id, $name, $title);
		$this->content_admin = _('Articles administration');
		$this->content_by = _('Article by');
		$this->content_item = _('Article');
		$this->content_items = _('Articles');
		$this->content_list_title = _('Article list');
		$this->content_list_title_by = _('Articles by');
		$this->content_list_count = 10;
		$this->content_list_order = 'title ASC';
		$this->content_title = _('Articles');
	}
}

?>
