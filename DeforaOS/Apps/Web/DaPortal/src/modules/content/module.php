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
//TODO:
//- complete paging



require_once('./system/module.php');


//ContentModule
class ContentModule extends Module
{
	//public
	//methods
	//essential
	//ContentModule::ContentModule
	public function __construct($id, $name)
	{
		parent::__construct($id, $name);
		$this->module_name = _('Content');
		$this->module_content = _('Content');
		$this->module_content_submit = _('Submit content');
		$this->module_contents = _('Content');
	}


	//useful
	//ContentModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'disable':
			case 'enable':
			case 'submit':
				return $this->$action($engine, $request);
			case 'list':
				return $this->_list($engine, $request);
			case 'preview':
				return $this->preview($engine,
						$request->getId(),
						$request->getTitle());
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
	//properties
	protected $module_id = FALSE;
	protected $module_name = 'Content';
	protected $module_content = 'Content';
	protected $module_content_submit = 'Submit content';
	protected $module_contents = 'Content';

	protected $content_list_count = 10;
	protected $content_list_order = 'timestamp DESC';

	//queries
	protected $query_admin_delete = 'DELETE FROM daportal_content
		WHERE content_id=:content_id';
	protected $query_admin_disable = "UPDATE daportal_content
		SET enabled='0'
		WHERE content_id=:content_id";
	protected $query_admin_enable = "UPDATE daportal_content
		SET enabled='1'
		WHERE content_id=:content_id";
	protected $query_delete = 'DELETE FROM daportal_content
		WHERE content_id=:content_id AND user_id=:user_id';
	protected $query_disable = "UPDATE daportal_content
		SET enabled='0'
		WHERE content_id=:content_id AND user_id=:user_id";
	protected $query_enable = "UPDATE daportal_content
		SET enabled='1'
		WHERE content_id=:content_id AND user_id=:user_id";
	protected $query_get = "SELECT daportal_module.name AS module,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content AS text,
		timestamp AS date
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND content_id=:id";
	protected $query_list = "SELECT content_id AS id, timestamp AS date,
		name AS module, daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_list_admin = "SELECT content_id AS id,
		timestamp AS date, name AS module,
		daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_list_user = "SELECT content_id AS id,
		timestamp AS date, name AS module,
		daportal_user.user_id AS user_id, username, title,
	       	daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_user.user_id=:user_id";


	//methods
	//accessors
	//ContentModule::canSubmit
	protected function canSubmit($engine, $request = FALSE)
	{
		global $config;
		$cred = $engine->getCredentials();

		if($cred->getUserId() > 0)
			return TRUE;
		if($config->getValue('module::'.$this->name, 'anonymous'))
			return TRUE;
		return FALSE;
	}


	//useful
	//ContentModule::admin
	protected function admin($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		if(!$cred->isAdmin())
		{
			$error = _('Permission denied');
			$engine->log('LOG_ERR', $error);
			$r = new Request($engine, 'user', 'login');
			$dialog = $page->append('dialog', array(
						'type' => 'error',
						'text' => $error));
			$dialog->append('button', array('stock' => 'login',
						'text' => _('Login'),
						'request' => $r));
			return $page;
		}
		$title = $this->module_name.' administration';
		$page->setProperty('title', $title);
		$element = $page->append('title', array('stock' => 'admin',
				'text' => $title));
		$db = $engine->getDatabase();
		$query = $this->query_list_admin;
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		if(($res = $db->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Unable to list contents')));
		$r = new Request($engine, $this->name, 'admin');
		$treeview = $page->append('treeview', array('request' => $r));
		$treeview->setProperty('columns', array('title', 'enabled',
					'username', 'date'));
		$toolbar = $treeview->append('toolbar');
		$toolbar->append('button', array('stock' => 'refresh',
					'text' => _('Refresh'),
					'request' => $r));
		$toolbar->append('button', array('stock' => 'disable',
					'text' => _('Disable'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'disable'));
		$toolbar->append('button', array('stock' => 'enable',
					'text' => _('Enable'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'enable'));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('id', 'content_id:'.$res[$i]['id']);
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
		}
		return $page;
	}


	//ContentModule::_default
	protected function _default($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $this->query_list;
		$p = ($request !== FALSE) ? $request->getParameter('page') : 0;

		if($request !== FALSE && ($id = $request->getId()) !== FALSE)
			return $this->_display($engine, $id,
					$request->getTitle());
		$page = new Page;
		$page->append('title', array('stock' => $this->name,
				'text' => $this->module_name));
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		if(is_string(($order = $this->content_list_order)))
			$query .= ' ORDER BY '.$order;
		if(($limit = $this->content_list_count) > 0 && is_int($limit))
			$query .= ' LIMIT '.$limit;
		if(is_numeric($p) && $p > 1)
			$query .= ' OFFSET '.($limit * ($p - 1));
		if(($res = $db->query($engine, $query)) === FALSE)
		{
			$error = _('Unable to list contents');
			$page->append('dialog', array('type' => 'error',
						'text' => $error));
			return $page;
		}
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
			$page->appendElement($this->_preview($engine,
						$res[$i]['id']));
		return $page;
	}


	//ContentModule::delete
	protected function delete($engine, $request)
	{
		$query = $this->query_delete;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_delete;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be deleted successfully'),
				_('Some content could not be deleted'));
	}


	//ContentModule::disable
	protected function disable($engine, $request)
	{
		$query = $this->query_disable;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_disable;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be disabled successfully'),
				_('Some content could not be disabled'));
	}


	//ContentModule::enable
	protected function enable($engine, $request)
	{
		$query = $this->query_enable;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_enable;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be enabled successfully'),
				_('Some content could not be enabled'));
	}


	//ContentModule::list
	protected function _list($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$uid = ($request !== FALSE) ? $request->getId() : FALSE;

		$page = new Page;
		$title = $this->module_contents;
		if($uid !== FALSE)
			$title .= _(' by ').$uid; //XXX
		$page->setProperty('title', $title);
		$element = $page->append('title');
		$query = ($uid !== FALSE) ? $this->query_list_user
			: $this->query_list;
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		if(($res = $db->query($engine, $query, array(
							'user_id' => $uid)))
				=== FALSE)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Unable to list contents')));
		$element->setProperty('text', $title);
		$r = new Request($engine, $this->name, 'list', $uid);
		$treeview = $page->append('treeview', array('request' => $r));
		$columns = array('title' => _('Title'));
		if($uid === $cred->getUserId())
			$columns['enabled'] = _('Enabled');
		$columns['username'] = _('Username');
		$columns['date'] = _('Date');
		$treeview->setProperty('columns', $columns);
		$toolbar = $treeview->append('toolbar');
		$toolbar->append('button', array('stock' => 'refresh',
				'text' => _('Refresh'),
				'request' => $r));
		if($uid === $cred->getUserId())
		{
			$toolbar->append('button', array('stock' => 'disable',
						'text' => _('Disable'),
						'type' => 'submit',
						'name' => 'action',
						'value' => 'disable'));
			$toolbar->append('button', array('stock' => 'enable',
						'text' => _('Enable'),
						'type' => 'submit',
						'name' => 'action',
						'value' => 'enable'));
		}
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('id', 'content_id:'.$res[$i]['id']);
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
		}
		return $page;
	}


	//ContentModule::preview
	protected function preview($engine, $id, $title = FALSE)
	{
		return $this->_preview($engine, $id, $title);
	}


	//ContentModule::submit
	protected function submit($engine, $request = FALSE)
	{
		$title = $this->module_content_submit;
		$error = _('Permission denied');

		if(!$this->canSubmit($engine, $request, $error))
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//FIXME really implement
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title));
		return $page;
	}


	//private
	//methods
	//ContentModule::_apply
	private function _apply($engine, $request, $query, $fallback, $success,
			$failure)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(($uid = $cred->getUserId()) == 0)
		{
			//must be logged in
			$page = $this->_default($engine);
			$page->prepend('dialog', array('type' => 'error',
						'text' => _('Must be logged in')));
			return $page;
		}
		if($engine->isIdempotent($request))
			//must be safe
			return $this->$fallback($engine);
		$type = 'info';
		$message = $success;
		$parameters = $request->getParameters();
		foreach($parameters as $k => $v)
		{
			$x = explode(':', $k);
			if(count($x) != 2 || $x[0] != 'content_id'
					|| !is_numeric($x[1]))
				continue;
			$res = $db->query($engine, $query, array(
						'content_id' => $x[1],
						'user_id' => $uid));
			if($res !== FALSE)
				continue;
			$type = 'error';
			$message = $failure;
		}
		$page = $this->$fallback($engine);
		$page->prepend('dialog', array('type' => $type,
					'text' => $message));
		return $page;
	}


	//ContentModule::_display
	private function _display($engine, $id, $title = FALSE)
	{
		if($id === FALSE)
			return $this->default();
		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return FALSE;
		//FIXME display metadata and link to actual resource?
		$page = new Page;
		$page->setProperty('title', $content['title']);
		$page->append('title', array('stock' => $this->name,
				'text' => $content['title']));
		$element = $page->append('label');
		$element->setProperty('text', $content['text']);
		return $page;
	}


	//ContentModule::_get
	private function _get($engine, $id, $title = FALSE)
	{
		if($id === FALSE)
			return FALSE;
		$db = $engine->getDatabase();
		$query = $this->query_get;
		$args = array('id' => $id);
		if($title !== FALSE)
		{
			$query .= ' AND title LIKE :title';
			$args['title'] = str_replace('-', '_', $title);
		}
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}

	
	//ContentModule::_preview
	private function _preview($engine, $id, $title = FALSE)
	{
		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return new PageElement('dialog', array(
						'type' => 'error',
						'text' => 'Could not fetch content'));
		$page = new PageElement('vbox');
		$r = new Request($engine, $content['module'], FALSE,
				$content['id']);
		$title = $page->append('title');
		$title->append('link', array('request' => $r,
					'text' => $content['title']));
		$page->append('label', array('text' => $this->module_content
					._(' by ').$content['username']
					._(' on ').$content['date']));
		$hbox = $page->append('hbox');
		$hbox->append('image', array('stock' => 'module '
					.$content['module'].' content'));
		$hbox->append('label', array('text' => $content['text']));
		$page->append('button', array('stock' => 'read',
					'text' => _('Read'), 'request' => $r));
		return $page;
	}
}

?>
