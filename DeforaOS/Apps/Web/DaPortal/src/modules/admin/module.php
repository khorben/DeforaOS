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
	//AdminModule::AdminModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('Administration') : $title;
		parent::__construct($id, $name, $title);
	}


	//useful
	//AdminModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
		{
			if($request->getAction() == 'actions')
				return FALSE;
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('Permission denied')));
		}
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'actions':
				return $this->$action($engine, $request);
			case 'admin':
			case 'default':
			case 'disable':
			case 'enable':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return FALSE;
	}


	//protected
	//properties
	//queries
	protected $query_admin = "SELECT name FROM daportal_module
		WHERE enabled='1' ORDER BY name ASC";
	protected $query_admin_modules = "SELECT module_id, name, enabled
		FROM daportal_module
		ORDER BY name ASC";
	protected $query_disable = "UPDATE daportal_module
		SET enabled='0'
		WHERE module_id=:module_id";
	protected $query_enable = "UPDATE daportal_module
		SET enabled='1'
		WHERE module_id=:module_id";


	//methods
	//useful
	//actions
	//AdminModule::actions
	protected function actions($engine, $request)
	{
		if($request->getParameter('user') !== FALSE)
			return FALSE;
		$r = new Request($engine, $this->name, 'admin');
		$icon = new PageElement('image', array('stock' => 'admin'));
		$link = new PageElement('link', array('request' => $r,
				'text' => _('Modules')));
		$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		return $ret;
	}


	//calls
	//AdminModule::callAdmin
	protected function callAdmin($engine, $request = FALSE)
	{
		$title = _('Modules administration');
		$query = $this->query_admin_modules;
		$database = $engine->getDatabase();
		$actions = array('disable', 'enable');

		//perform actions if necessary
		if($request !== FALSE)
			foreach($actions as $a)
				if($request->getParameter($a) !== FALSE)
				{
					$a = 'call'.ucfirst($a);
					return $this->$a($engine, $request);
				}
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
			'size' => 16, 'title' => _('Disabled')));
		$yes = new PageElement('image', array('stock' => 'yes',
			'size' => 16, 'title' => _('Enabled')));
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


	//AdminModule::callDefault
	protected function callDefault($engine, $request)
	{
		$title = _('Administration');
		$database = $engine->getDatabase();
		$query = $this->query_admin;

		//obtain the list of modules
		if(($res = $database->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$page = new Page(array('title' => $title));
		//title
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$vbox = $page->append('vbox');
		$vbox->append('title'); //XXX to reduce the next level of titles
		$vbox = $vbox->append('vbox');
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
				$view->append($r);
		}
		return $page;
	}


	//AdminModule::callDisable
	protected function callDisable($engine, $request)
	{
		$query = $this->query_disable;

		return $this->helperApply($engine, $request, $query, 'admin',
				_('Module(s) could be disabled successfully'),
				_('Some module(s) could not be disabled'));
	}


	//AdminModule::callEnable
	protected function callEnable($engine, $request)
	{
		$query = $this->query_enable;

		return $this->helperApply($engine, $request, $query, 'admin',
				_('Module(s) could be enabled successfully'),
				_('Some module(s) could not be enabled'));
	}


	//helpers
	//AdminModule::helperApply
	protected function helperApply($engine, $request, $query, $fallback,
			$success, $failure)
	{
		//XXX copied from ContentModule
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(!$cred->isAdmin())
		{
			//must be admin
			$page = $this->callDefault($engine);
			$error = _('Permission denied');
			$page->prepend('dialog', array('type' => 'error',
				'text' => $error));
			return $page;
		}
		$fallback = 'call'.ucfirst($fallback);
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
}

?>
