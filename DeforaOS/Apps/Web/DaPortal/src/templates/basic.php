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
	protected $footer;
	protected $homepage = '/';
	protected $title;


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


	//BasicTemplate::getTitle
	protected function getTitle($engine)
	{
		$title = new PageElement('title');
		$title->append('link', array('text' => $this->title,
					'request' => new Request($engine)));
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
		$this->footer = $engine->getConfig('template::basic',
				'footer');
		if(($homepage = $engine->getConfig('template::basic', 'footer'))
				!== FALSE)
			$this->homepage = $homepage;
		if(($this->title = $engine->getConfig('template::basic',
						'title')) === FALSE)
			$this->title = $engine->getConfig(FALSE, 'title');
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
