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



require_once('./templates/basic.php');


//DeforaOSTemplate
class DeforaOSTemplate extends BasicTemplate
{
	//protected
	//methods
	//accessors
	//DeforaOSTemplate::getMenu
	protected function getMenu($engine, $entries = FALSE)
	{
		$cred = $engine->getCredentials();

		//obtain the parent menu
		if($entries === FALSE)
			$entries = $this->getEntries($engine);
		if(($menu = parent::getMenu($engine, $entries)) === FALSE)
			return FALSE;
		$vbox = new PageElement('vbox', array('id' => 'menu'));
		$vbox->append($menu);
		//add some widgets
		//search widget
		$request = new Request('search', 'widget');
		if(($widget = $engine->process($request)) !== FALSE)
			$vbox->append($widget);
		//user widget
		$request = new Request('user', 'widget');
		if(($widget = $engine->process($request)) !== FALSE)
			$vbox->append($widget);
		return $vbox;
	}


	//DeforaOSTemplate::getTitle
	protected function getTitle($engine)
	{
		$title = new PageElement('title', array('id' => 'title'));
		$link = $title->append('link', array('url' => $this->homepage));
		$link->append('image', array(
			'source' => 'themes/DeforaOS.png'));
		return $title;
	}


	//useful
	//DeforaOSTemplate::match
	protected function match($engine)
	{
		return 0;
	}


	//DeforaOSTemplate::attach
	protected function attach($engine)
	{
		$this->name = 'DeforaOS';
		parent::attach($engine);
	}
}

?>
