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



require_once('./system/template.php');


//BasicTemplate
class BasicTemplate extends Template
{
	//protected
	//properties
	protected $name = 'basic';
	protected $footer = FALSE;
	protected $homepage = FALSE;
	protected $title = FALSE;


	//methods
	//accessors
	protected function getFooter($engine)
	{
		$footer = new PageElement('statusbar');
		$footer->setProperty('id', 'footer');
		if($this->footer !== FALSE)
			$footer->setProperty('text', $this->footer);
		return $footer;
	}


	//BasicTemplate::getMenu
	protected function getMenu($engine)
	{
		$menu = new PageElement('menubar');
		//FIXME implement
		return $menu;
	}


	//BasicTemplate::getTitle
	protected function getTitle($engine)
	{
		$title = new PageElement('title');
		$title->setProperty('id', 'title');
		$title->append('link', array('text' => $this->title,
					'url' => $this->homepage));
		return $title;
	}


	//useful
	//BasicTemplate::match
	protected function match(&$engine)
	{
		return 100;
	}


	//BasicTemplate::attach
	protected function attach(&$engine)
	{
		global $config;

		$section = 'template::'.$this->name;
		if($this->footer === FALSE)
			$this->footer = $config->getVariable($section,
					'footer');
		if($this->homepage === FALSE)
			$this->homepage = $config->getVariable($section,
					'footer');
		if($this->title === FALSE)
			$this->title = $config->getVariable($section, 'title');
		if($this->title === FALSE)
			$this->title = $config->getVariable(FALSE, 'title');
	}


	//BasicTemplate::render
	public function render(&$engine, $page)
	{
		if($page === FALSE)
			$page = new Page;
		$page->prependElement($this->getMenu($engine));
		$page->prependElement($this->getTitle($engine));
		$page->setProperty('type', 'block');
		$page->appendElement($this->getFooter($engine));
		return $page;
	}
}

?>
