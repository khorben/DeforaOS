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


//WikiModule
class WikiModule extends ContentModule
{
	//public
	//methods
	//essential
	//WikiModule::WikiModule
	public function __construct($id, $name, $title = FALSE)
	{
		global $config;

		$this->root = $config->getVariable('module::wiki', 'root');
		$title = ($title === FALSE) ? _('Wiki') : $title;
		parent::__construct($id, $name);
		$this->text_content_admin = _('Wiki administration');
		$this->text_content_by = _('Page by');
		$this->text_content_item = _('Wiki page');
		$this->text_content_items = _('Wiki pages');
		$this->text_content_list_title = _('Wiki pages');
		$this->text_content_list_title_by = _('Wiki pages by');
		$this->text_content_submit = _('New wiki page');
		$this->text_content_title = _('Wiki');
	}


	//protected
	//methods
	//calls
	//WikiModule::callDefault
	protected function callDefault($engine, $request = FALSE)
	{
		$title = $this->text_content_title;

		if($request !== FALSE && $request->getId() !== FALSE)
			return $this->callDisplay($engine, $request);
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title,
				'stock' => $this->name));
		$vbox = $page->append('vbox');
		//search
		$vbox->append('title', array('text' => _('Search the wiki'),
				'stock' => 'search'));
		$r = new Request($engine, 'search', 'advanced', FALSE, FALSE,
				array('inmodule' => $this->name, //XXX
				'intitle' => 1));
		$form = $vbox->append('form', array('request' => $r));
		$hbox = $form->append('hbox');
		$hbox->append('entry', array('name' => 'q',
				'text' => _('Look for a page: ')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'search', 'text' => _('Search')));
		$r = new Request($engine, 'search', 'advanced', FALSE, FALSE,
				array('inmodule' => $this->name, //XXX
				'incontent' => 1));
		$form = $vbox->append('form', array('request' => $r));
		$hbox = $form->append('hbox');
		$hbox->append('entry', array('name' => 'q',
				'text' => _('Look inside pages: ')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'search', 'text' => _('Search')));
		$vbox->append('title', array('text' => _('Recent changes'),
				'stock' => 'help'));
		//recent changes
		$vbox->append($this->callHeadline($engine, FALSE));
		return $page;
	}


	//helpers
	//WikiModule::helperDisplayText
	protected function helperDisplayText($engine, $page, $request, $content)
	{
		$error = _('Could not display page');

		if($this->root === FALSE
				|| strpos($content['title'], '/') !== FALSE)
		{
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
			return;
		}
		//obtain the page
		$cmd = 'co -p -q '.escapeshellarg(
				$this->root.'/'.$content['title']);
		exec($cmd, $rcs, $res);
		if($res != 0)
		{
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
			return;
		}
		$rcs = implode("\n", $rcs);
		$page->append('htmlview', array('text' => $rcs));
	}


	//private
	//properties
	private $root = FALSE;
}

?>
