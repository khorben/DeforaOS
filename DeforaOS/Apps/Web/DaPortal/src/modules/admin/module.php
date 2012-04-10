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



require_once('./system/module.php');


//AdminModule
class AdminModule extends Module
{
	//public
	//methods
	//essential
	//AdminModule::call
	public function call(&$engine, $request)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('Permission denied')));
		switch(($action = $request->getAction($request)))
		{
			case 'actions':
			case 'admin':
				return $this->$action($engine, $request);
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
	//properties
	//queries
	protected $query_admin = "SELECT name FROM daportal_module
		WHERE enabled='1' ORDER BY name ASC";
	protected $query_admin_modules = "SELECT module_id, name, enabled
		FROM daportal_module
		ORDER BY name ASC";


	//methods
	//actions
	//AdminModule::actions
	protected function actions($engine, $request)
	{
		$r = new Request($engine, $this->name, 'admin');
		$icon = new PageElement('image', array('stock' => 'admin'));
		$link = new PageElement('link', array('request' => $r,
				'text' => _('Modules')));
		$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		return $ret;
	}


	//AdminModule::admin
	protected function admin($engine, $request)
	{
		$title = _('Modules administration');
		$query = $this->query_admin_modules;
		$database = $engine->getDatabase();

		if(($res = $database->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'admin',
			'text' => $title));
		$view = $page->append('treeview');
		$toolbar = $view->append('toolbar');
		$r = new Request($engine, $this->name, 'admin');
		$toolbar->append('button', array('request' => $r,
		       	'stock' => 'refresh',
			'text' => _('Refresh')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $view->append('row');
			//FIXME really implement
			$r = new Request($engine, $res[$i]['name'], 'actions',
					FALSE, FALSE, array('admin' => TRUE));
			$text = ucfirst($res[$i]['name']);
			$link = new PageElement('link', array('request' => $r,
				'stock' => $res[$i]['name'],
				'text' => $text));
			$row->setProperty('title', $link);
		}
		$r = new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back to the administration')));
		return $page;
	}


	//AdminModule::_default
	protected function _default($engine, $request)
	{
		$title = _('Administration');
		$query = $this->query_admin;
		$database = $engine->getDatabase();

		if(($res = $database->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$vbox = $page->append('vbox');
		$vbox->append('title'); //XXX to reduce the next level of titles
		$vbox = $vbox->append('vbox');
		//FIXME output one iconview per module instead
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$r = new Request($engine, $res[$i]['name'], 'actions',
					FALSE, FALSE, array('admin' => TRUE));
			$rows = $engine->process($r);
			if(!is_array($rows) || count($rows) == 0)
				continue;
			$text = ucfirst($res[$i]['name']);
			$vbox->append('title', array(
					'stock' => $res[$i]['name'],
					'text' => $text));
			$view = $vbox->append('iconview');
			foreach($rows as $r)
				$view->appendElement($r);
		}
		return $page;
	}
}

?>
