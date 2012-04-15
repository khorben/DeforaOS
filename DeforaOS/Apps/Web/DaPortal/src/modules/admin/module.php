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
			case 'disable':
			case 'enable':
				return $this->$action($engine, $request);
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
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
	protected function admin($engine, $request = FALSE)
	{
		$title = _('Modules administration');
		$query = $this->query_admin_modules;
		$database = $engine->getDatabase();
		$actions = array('disable', 'enable');

		//perform actions if necessary
		if($request !== FALSE)
			foreach($actions as $a)
				if($request->getParameter($a) !== FALSE)
					return $this->$a($engine, $request);
		//list modules
		if(($res = $database->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'admin',
			'text' => $title));
		$r = new Request($engine, $this->name, 'admin');
		$columns = array('module' => _('Module'),
				'enabled' => _('Enabled'));
		$view = $page->append('treeview', array('request' => $r,
				'columns' => $columns));
		//toolbar
		$toolbar = $view->append('toolbar');
		$toolbar->append('button', array('request' => $r,
				'stock' => 'refresh',
				'text' => _('Refresh')));
		$toolbar->append('button', array('stock' => 'disable',
				'text' => _('Disable'),
				'type' => 'submit', 'name' => 'action',
				'value' => 'disable'));
		$toolbar->append('button', array('stock' => 'enable',
				'text' => _('Enable'),
				'type' => 'submit', 'name' => 'action',
				'value' => 'enable'));
		$no = new PageElement('image', array('stock' => 'no',
			'size' => 16));
		$yes = new PageElement('image', array('stock' => 'yes',
			'size' => 16));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $view->append('row');
			$row->setProperty('id', 'module_id:'
					.$res[$i]['module_id']);
			$r = new Request($engine, $res[$i]['name'], 'admin');
			$text = ucfirst($res[$i]['name']);
			$link = new PageElement('link', array('request' => $r,
				'stock' => $res[$i]['name'],
				'text' => $text));
			$row->setProperty('module', $link);
			$row->setProperty('enabled', $database->isTrue(
					$res[$i]['enabled']) ? $yes : $no);
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


	//AdminModule::disable
	protected function disable($engine, $request)
	{
		$query = $this->query_disable;

		return $this->_apply($engine, $request, $query, 'admin',
			_('Module(s) could be disabled successfully'),
			_('Some module(s) could not be disabled'));
	}


	//AdminModule::enable
	protected function enable($engine, $request)
	{
		$query = $this->query_enable;

		return $this->_apply($engine, $request, $query, 'admin',
			_('Module(s) could be enabled successfully'),
			_('Some module(s) could not be enabled'));
	}


	//helpers
	//AdminModule::apply
	protected function _apply($engine, $request, $query, $fallback,
			$success, $failure)
	{
		//XXX copied from ContentModule
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(!$cred->isAdmin())
		{
			//must be admin
			$page = $this->_default($engine);
			$error = _('Permission denied');
			$page->prepend('dialog', array('type' => 'error',
				'text' => $error));
			return $page;
		}
		if($request->isIdempotent())
			//must be safe
			return $this->$fallback($engine);
		$type = 'info';
		$message = $success;
		$parameters = $request->getParameters();
		foreach($parameters as $k => $v)
		{
			$x = explode(':', $k);
			if(count($x) != 2 || $x[0] != 'module_id'
					|| !is_numeric($x[1]))
				continue;
			$res = $db->query($engine, $query, array(
					'module_id' => $x[1]));
			if($res !== FALSE)
				continue;
			$type = 'error';
			$message = $failure;
		}
		$page = $this->$fallback($engine);
		//FIXME place this under the title
		$page->prepend('dialog', array('type' => $type,
				'text' => $message));
		return $page;
	}


	//private
	//queries
	private $query_admin = "SELECT name FROM daportal_module
		WHERE enabled='1' ORDER BY name ASC";
	private $query_admin_modules = "SELECT module_id, name, enabled
		FROM daportal_module
		ORDER BY name ASC";
	private $query_disable = "UPDATE daportal_module
		SET enabled='0'
		WHERE module_id=:module_id";
	private $query_enable = "UPDATE daportal_module
		SET enabled='1'
		WHERE module_id=:module_id";
}

?>
