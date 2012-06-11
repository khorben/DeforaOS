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
		//settings
		$this->content_list_count = 10;
		$this->content_list_order = 'title ASC';
		//translations
		$this->text_content_admin = _('Articles administration');
		$this->text_content_by = _('Article by');
		$this->text_content_item = _('Article');
		$this->text_content_items = _('Articles');
		$this->text_content_list_title = _('Article list');
		$this->text_content_list_title_by = _('Articles by');
		$this->text_content_more_content = _('More articles...');
		$this->text_content_title = _('Articles');
	}
}

?>
