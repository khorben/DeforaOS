<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require_once('./system/module.php');


//SearchModule
class SearchModule extends Module
{
	//public
	//methods
	//SearchModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'actions':
				return $this->actions($engine, $request);
			case 'admin':
			case 'advanced':
			case 'default':
			case 'widget':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return FALSE;
	}


	//protected
	//properties
	protected $query = "FROM daportal_content, daportal_module,
		daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_fields = 'SELECT content_id AS id, timestamp AS date,
		name AS module, daportal_content.user_id AS user_id, title,
		content, username';


	//methods
	//useful
	//SearchModule::actions
	protected function actions($engine, $request)
	{
		if($request->getParameter('admin') !== FALSE)
			return FALSE;
		if($request->getParameter('user') !== FALSE)
			return FALSE;
		$ret = array();
		//advanced search
		$r = new Request($this->name, 'advanced');
		$ret[] = $this->helperAction($engine, 'add', $r,
				_('Advanced search'));
		return $ret;
	}


	//SearchModule::appendResult
	protected function appendResult(&$engine, &$view, &$res)
	{
		$row = $view->append('row');
		$row->setProperty('title', $res['title']);
		$row->setProperty('username', $res['username']);
		$row->setProperty('date', $res['date']);
		$r = new Request($res['module'], 'preview', $res['id'],
				$res['title']);
		$row->setProperty('preview', $engine->process($r));
	}


	//calls
	//SearchModule::callAdmin
	protected function callAdmin(&$engine)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
			return $engine->log('LOG_ERR', 'Permission denied');
		$title = _('Search administration');
		//FIXME implement settings
		return FALSE;
	}


	//SearchModule::callDefault
	protected function callDefault(&$engine, $request)
	{
		$page = $this->pageSearch($engine, $request);
		if(($q = $request->getParameter('q')) === FALSE
				|| strlen($q) == 0)
			return $page;
		$count = 0;
		$res = $this->query($engine, $q, $count, TRUE, TRUE);
		$results = $page->append('vbox');
		$results->setProperty('id', 'search_results');
		$label = $results->append('label');
		$label->setProperty('text', $count.' result(s)');
		$columns = array('title', 'username', 'date', 'preview');
		$view = $page->append('treeview', array('view' => 'preview',
					'columns' => $columns));
		for($i = 0; $i < $count; $i++)
			$this->appendResult($engine, $view, $res[$i]);
		//FIXME implement paging
		return $page;
	}


	//SearchModule::callAdvanced
	protected function callAdvanced(&$engine, $request)
	{
		$page = $this->pageSearch($engine, $request, TRUE);
		if(($q = $request->getParameter('q')) === FALSE
				|| strlen($q) == 0)
			return $page;
		$count = 0;
		$intitle = $request->getParameter('intitle');
		$incontent = $request->getParameter('incontent');
		if($intitle === FALSE && $incontent === FALSE)
			$intitle = $incontent = TRUE;
		$res = $this->query($engine, $q, $count, $intitle, $incontent);
		$results = $page->append('vbox');
		$results->setProperty('id', 'search_results');
		$label = $results->append('label');
		$label->setProperty('text', $count.' result(s)');
		$columns = array('title', 'username', 'date', 'preview');
		$view = $page->append('treeview', array('view' => 'preview',
					'columns' => $columns));
		for($i = 0; $i < $count; $i++)
			$this->appendResult($engine, $view, $res[$i]);
		return $page;
	}


	//SearchModule::callWidget
	protected function callWidget(&$engine, $request)
	{
		$form = new PageElement('form', array('idempotent' => TRUE));
		$r = new Request('search');
		$form->setProperty('request', $r);
		$hbox = $form->append('hbox');
		$entry = $hbox->append('entry');
		$entry->setProperty('name', 'q');
		$entry->setProperty('value', _('Search...'));
		$button = $hbox->append('button', array('stock' => 'search',
					'type' => 'submit',
					'text' => _('Search'),
					'autohide' => TRUE));
		return $form;
	}


	//helpers
	//SearchModule::helperAction
	protected function helperAction($engine, $stock, $request, $text)
	{
		$icon = new PageElement('image', array('stock' => $stock));
		$link = new PageElement('link', array('request' => $request,
				'text' => $text));
		return new PageElement('row', array('icon' => $icon,
				'label' => $link));
	}


	//SearchModule::pageSearch
	protected function pageSearch(&$engine, $request, $advanced = FALSE)
	{
		$q = $request->getParameter('q');
		$page = new Page;
		$page->setProperty('title', _('Search'));
		$title = $page->append('title', array('stock' => 'search'));
		$title->setProperty('text', $q ? _('Search results')
				: _('Search'));
		$form = $page->append('form');
		$r = new Request('search', $advanced ? 'advanced' : FALSE);
		$form->setProperty('request', $r);
		$entry = $form->append('entry');
		$entry->setProperty('text', _('Search query: '));
		$entry->setProperty('name', 'q');
		$entry->setProperty('value', $request->getParameter('q'));
		if($advanced)
		{
			$hbox = $form->append('hbox');
			$label = $hbox->append('label');
			$label->setProperty('text', _('Search in: '));
			$checkbox = $hbox->append('checkbox');
			$checkbox->setProperty('name', 'intitle');
			$checkbox->setProperty('text', _('titles'));
			$checkbox->setProperty('value',
					$request->getParameter('intitle'));
			$checkbox = $hbox->append('checkbox');
			$checkbox->setProperty('name', 'incontent');
			$checkbox->setProperty('text', _('content'));
			$checkbox->setProperty('value',
					$request->getParameter('incontent'));
			$button = $form->append('button', array(
						'type' => 'reset',
						'text' => _('Reset')));
		}
		$button = $form->append('button', array('stock' => 'search',
					'type' => 'submit',
					'text' => _('Search')));
		$link = $page->append('link');
		if($advanced)
		{
			$link->setProperty('stock', 'remove');
			$link->setProperty('text', _('Simpler search...'));
			$link->setProperty('request', new Request('search',
				FALSE, FALSE, FALSE,
				$q ? array('q' => $q) : FALSE));
		}
		else
		{
			$link->setProperty('stock', 'add');
			$link->setProperty('text', _('Advanced search...'));
			$link->setProperty('request', new Request('search',
				'advanced', FALSE, FALSE,
				$q ? array('q' => $q) : FALSE));
		}
		return $page;
	}


	//SearchModule::query
	protected function query($engine, $string, &$count, $intitle,
			$incontent, $user = FALSE, $module = FALSE)
	{
		global $db;

		$db = $engine->getDatabase();
		$query = $this->query.' AND (0=1';
		$q = explode(' ', $string);
		$args = array();
		$i = 0;
		if($intitle && count($q))
			foreach($q as $r)
			{
				$query .= " OR title LIKE :arg$i";
				$args['arg'.$i++] = "%$r%";
			}
		if($incontent && count($q))
			foreach($q as $r)
			{
				$query .= " OR content LIKE :arg$i";
				$args['arg'.$i++] = "%$r%";
			}
		$query .= ')';
		$fields = 'SELECT COUNT (*)';
		if(($res = $db->query($engine, $fields.' '.$query, $args))
				=== FALSE)
			return $engine->log('LOG_ERR', _('Unable to search'));
		$count = $res[0][0];
		$fields = $this->query_fields;
		$order = 'ORDER BY timestamp DESC';
		//FIXME also set a limit
		if(($res = $db->query($engine, $fields.' '.$query.' '.$order,
					$args)) === FALSE)
			return $engine->log('LOG_ERR', _('Unable to search'));
		return $res;
	}
}

?>
