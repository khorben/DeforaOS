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
require_once('./modules/content/module.php');


//NewsModule
class NewsModule extends ContentModule
{
	//public
	//methods
	//essential
	//NewsModule::NewsModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('News') : FALSE;
		parent::__construct($id, $name, $title);
		//translations
		$this->text_content_admin = _('News administration');
		$this->text_content_by = _('News by');
		$this->text_content_item = _('News item');
		$this->text_content_items = _('News items');
		$this->text_content_list_title = _('News list');
		$this->text_content_list_title_by = _('News by');
		$this->text_content_more_content = _('More news...');
		$this->text_content_submit = _('Submit news...');
		$this->text_content_title = _('News');
	}


	//useful
	//NewsModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		switch($request->getAction())
		{
			case 'rss':
				//for backward compatibility
				return $this->callRss($engine, $request);
		}
		return parent::call($engine, $request);
	}


	//calls
	//NewsModule::callRss
	protected function callRss($engine, $request)
	{
		$engine->setType('application/rss+xml');
		return $this->callHeadline($engine, $request);
	}


	//helpers
	//NewsModule::helperDisplayText
	protected function helperDisplayText($engine, $page, $request, $content)
	{
		$text = $content['content'];

		$text = HTML::format($engine, $text);
		$page->append('htmlview', array('text' => $text));
	}
}

?>
