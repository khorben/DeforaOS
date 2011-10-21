<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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
	public function call(&$engine, $request)
	{
		switch($request->getAction())
		{
			case 'admin':
				return $this->admin($engine);
			case 'advanced':
				return $this->searchAdvanced($engine, $request);
			default:
				return $this->search($engine, $request);
		}
	}


	//protected
	//methods
	//SearchModule::admin
	protected function admin(&$engine)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
			return $engine->log('LOG_ERR', 'Permission denied');
		$title = 'Search administration';
		//FIXME implement settings
		return FALSE;
	}


	//SearchModule::search
	protected function search(&$engine, $request)
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
		$treeview = $results->append('treeview');
		$treeview->setProperty('view', 'preview');
		$treeview->setProperty('columns', array('title', 'username',
					'date'));
		for($i = 0; $i < $count; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
			$row->setProperty('preview', $res[$i]['content']);
		}
		return $page;
	}


	//SearchModule::searchAdvanced
	protected function searchAdvanced(&$engine, $request)
	{
		$page = $this->pageSearch($engine, $request, TRUE);
		if(($q = $request->getParameter('q')) === FALSE
				|| strlen($q) == 0)
			return $page;
		$count = 0;
		$res = $this->query($engine, $q, $count,
				$request->getParameter('intitle'),
				$request->getParameter('incontent'));
		$results = $page->append('vbox');
		$results->setProperty('id', 'search_results');
		$label = $results->append('label');
		$label->setProperty('text', $count.' result(s)');
		$treeview = $results->append('treeview');
		$treeview->setProperty('view', 'preview');
		$treeview->setProperty('columns', array('title', 'username',
					'date'));
		for($i = 0; $i < $count; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
			$row->setProperty('preview', $res[$i]['content']);
		}
		return $page;
	}


	//private
	//properties
	private $query = "FROM daportal_content, daportal_module, daportal_user
WHERE daportal_content.module_id=daportal_module.module_id
AND daportal_content.user_id=daportal_user.user_id
AND daportal_content.enabled='1'
AND daportal_module.enabled='1'
AND daportal_user.enabled='1'";


	//methods
	private function pageSearch(&$engine, $request, $advanced = FALSE)
	{
		$q = $request->getParameter('q');
		$page = new Page;
		$page->setProperty('title', 'Search');
		$title = $page->append('title');
		$title->setProperty('text', $q ? 'Search results' : 'Search');
		$form = $page->append('form');
		$r = new Request($engine, 'search', $advanced ? 'advanced'
				: FALSE);
		$form->setProperty('request', $r);
		$entry = $form->append('entry');
		$entry->setProperty('text', 'Search query: ');
		$entry->setProperty('name', 'q');
		$entry->setProperty('value', $request->getParameter('q'));
		if($advanced)
		{
			$hbox = $form->append('hbox');
			$label = $hbox->append('label');
			$label->setProperty('text', 'Search in: ');
			$checkbox = $hbox->append('checkbox');
			$checkbox->setProperty('name', 'intitle');
			$checkbox->setProperty('text', 'titles');
			$checkbox->setProperty('value',
					$request->getParameter('intitle'));
			$checkbox = $hbox->append('checkbox');
			$checkbox->setProperty('name', 'incontent');
			$checkbox->setProperty('text', 'content');
			$checkbox->setProperty('value',
					$request->getParameter('incontent'));
			$button = $form->append('button');
			$button->setProperty('type', 'reset');
			$button->setProperty('text', 'Reset');
		}
		$button = $form->append('button');
		$button->setProperty('class', 'search');
		$button->setProperty('type', 'submit');
		$button->setProperty('text', 'Search');
		$link = $page->append('link');
		if($advanced)
		{
			$link->setProperty('class', 'advanced');
			$link->setProperty('text', 'Simpler search...');
			$link->setProperty('request', new Request($engine,
						'search', FALSE, FALSE, FALSE,
						$q ? array('q' => $q) : FALSE));
		}
		else
		{
			$link->setProperty('class', 'simple');
			$link->setProperty('text', 'Advanced search...');
			$link->setProperty('request', new Request($engine,
						'search', 'advanced', FALSE,
						FALSE, $q ? array('q' => $q)
						: FALSE));
		}
		return $page;
	}


	//SearchModule::query
	private function query($engine, $string, &$count, $intitle, $incontent,
			$user = FALSE, $module = FALSE)
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
		if(($res = $db->query($engine, 'SELECT COUNT (*) '.$query,
					$args)) === FALSE)
			return $engine->log('LOG_ERR', 'Unable to search');
		$count = $res[0][0];
		$fields = 'SELECT content_id AS id, timestamp AS date,
name AS module, daportal_content.user_id AS user_id, title, content, username';
		$order = 'ORDER BY timestamp DESC';
		if(($res = $db->query($engine, $fields.' '.$query.' '.$order,
					$args)) === FALSE)
			return $engine->log('LOG_ERR', 'Unable to search');
		return $res;
	}
}

?>
