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
	protected $action = FALSE;
	protected $name = 'basic';
	protected $footer = FALSE;
	protected $homepage = FALSE;
	protected $id = FALSE;
	protected $module = FALSE;
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
	protected function getMenu($engine, $entries = FALSE)
	{
		$menu = new PageElement('menubar');
		//FIXME really implement
		$modules = $menu->append('menuitem', array(
					'text' => 'Modules'));
		if($entries === FALSE)
			$entries = array('blog', 'news',
				'user' => array('login'));
		foreach($entries as $k => $v)
		{
			if(is_array($v))
			{
				$r = new Request($engine, $k);
				$module = $modules->append('menuitem', array(
							'text' => ucfirst($k),
							'request' => $r));
				foreach($v as $a)
				{
					$r = new Request($engine, $k, $a);
					$module->append('menuitem', array(
								'text' => ucfirst($a),
								'request' => $r));
				}
			}
			else
			{
				$r = new Request($engine, $v);
				$modules->append('menuitem', array(
							'text' => ucfirst($v),
							'request' => $r));
			}
		}
		return $menu;
	}


	//BasicTemplate::getTitle
	protected function getTitle($engine)
	{
		$title = new PageElement('title', array('id' => 'title'));
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
		if($this->action === FALSE)
			$this->action = $config->getVariable($section,
					'action');
		if($this->footer === FALSE)
			$this->footer = $config->getVariable($section,
					'footer');
		if($this->homepage === FALSE)
			$this->homepage = $config->getVariable($section,
					'homepage');
		if($this->id === FALSE)
			$this->id = $config->getVariable($section, 'id');
		if($this->module === FALSE)
			$this->module = $config->getVariable($section,
					'module');
		if($this->title === FALSE)
			$this->title = $config->getVariable($section, 'title');
		if($this->title === FALSE)
			$this->title = $config->getVariable(FALSE, 'title');
	}


	//BasicTemplate::render
	public function render(&$engine, $page)
	{
		global $config;

		$p = new Page;
		$p->appendElement($this->getTitle($engine));
		$main = $p->append('vbox', array('id' => 'main'));
		$main->appendElement($this->getMenu($engine));
		$content = $main->append('vbox', array('id' => 'content'));
		if($page === FALSE && $this->module !== FALSE)
		{
			$request = new Request($engine, $this->module,
					$this->action, $this->id);
			$page = $engine->process($request);
		}
		if($page !== FALSE)
		{
			if(($title = $page->getProperty('title')) === FALSE)
				$title = $this->title;
			$content->appendElement($page);
		}
		else
		{
			$title = $config->getVariable(FALSE, 'title');
			//FIXME append default content
		}
		$p->setProperty('title', $title);
		$p->appendElement($this->getFooter($engine));
		return $p;
	}
}

?>
