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
		//translations
		$this->content_by = _('Content by');
		$this->content_item = _('Content');
		$this->content_items = _('Content');
		$this->content_list_title = _('Content list');
		$this->content_list_title_by = _('Content by');
		$this->content_more_content = _('More content...');
		$this->content_open_text = _('Read');
		$this->content_submit = _('Submit content');
		$this->content_title = _('Content');
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
			case 'display':
			case 'enable':
			case 'headline':
			case 'submit':
			case 'update':
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
	protected $module_name = 'Content';

	protected $content_by = 'Content by';
	protected $content_item = 'Content';
	protected $content_items = 'Content';
	protected $content_list_count = 10;
	protected $content_list_order = 'timestamp DESC';
	protected $content_list_title = 'Content list';
	protected $content_list_title_by = 'Content by';
	protected $content_more_content = 'More content...';
	protected $content_open_stock = 'read';
	protected $content_open_text = 'Read';
	protected $content_preview_length = 150;
	protected $content_submit = 'Submit content';
	protected $content_title = 'Content';

	//queries
	protected $query_admin_delete = 'DELETE FROM daportal_content
		WHERE module_id=:module_id
		AND content_id=:content_id';
	protected $query_admin_disable = "UPDATE daportal_content
		SET enabled='0'
		WHERE module_id=:module_id
		AND content_id=:content_id";
	protected $query_admin_enable = "UPDATE daportal_content
		SET enabled='1'
		WHERE module_id=:module_id
		AND content_id=:content_id";
	protected $query_delete = 'DELETE FROM daportal_content
		WHERE module_id=:module_id
		AND content_id=:content_id
		AND user_id=:user_id';
	protected $query_disable = "UPDATE daportal_content
		SET enabled='0'
		WHERE module_id=:module_id
		AND content_id=:content_id AND user_id=:user_id";
	protected $query_enable = "UPDATE daportal_content
		SET enabled='1'
		WHERE module_id=:module_id
		AND content_id=:content_id AND user_id=:user_id";
	protected $query_get = "SELECT daportal_module.name AS module,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content, timestamp
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND content_id=:content_id";
	protected $query_list = "SELECT content_id AS id, timestamp,
		name AS module, daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_list_admin = "SELECT content_id AS id, timestamp,
		name AS module, daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_list_count = "SELECT COUNT(*)
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $query_list_user = "SELECT content_id AS id, timestamp,
		name AS module, daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
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


	//ContentModule::_get
	protected function _get($engine, $id, $title = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $this->query_get;

		if($id === FALSE)
			return FALSE;
		$args = array('module_id' => $this->id, 'content_id' => $id);
		if($title !== FALSE)
		{
			$query .= ' AND title LIKE :title';
			$args['title'] = str_replace('-', '_', $title);
		}
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return FALSE;
		$res = $res[0];
		$res['date'] = $this->_timestampToDate($res['timestamp'],
				_('d/m/Y H:i'));
		return $res;
	}


	//convertors
	//ContentModule::_timestampToDate
	//FIXME move to the SQL module?
	protected function _timestampToDate($timestamp, $format = 'd/m/Y H:i:s')
	{
		$date = substr($timestamp, 0, 19);
		$date = strtotime($date);
		$date = date($format, $date);
		return $date;
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
		if(($res = $db->query($engine, $query, array(
					'module_id' => $this->id))) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Unable to list contents')));
		$r = new Request($engine, $this->name, 'admin');
		$treeview = $page->append('treeview', array('request' => $r));
		$treeview->setProperty('columns', array('title' => _('Title'),
				'enabled' => _('Enabled'),
				'username' => _('Username'),
				'date' => _('Date')));
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
			$r = new Request($engine, $this->name, 'update',
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$date = $this->_timestampToDate(_('d/m/Y H:i'),
					$res[$i]['timestamp']);
			$row->setProperty('date', $date);
		}
		$r = new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Leave the administration page')));
		return $page;
	}


	//ContentModule::_apply
	protected function _apply($engine, $request, $query, $fallback,
			$success, $failure)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(($uid = $cred->getUserId()) == 0)
		{
			//must be logged in
			$page = $this->_default($engine);
			$error = _('Must be logged in');
			$page->prepend('dialog', array('type' => 'error',
						'text' => $error));
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
						'module_id' => $this->id,
						'content_id' => $x[1],
						'user_id' => $uid));
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


	//ContentModule::_default
	protected function _default($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$query = '';
		$p = ($request !== FALSE) ? $request->getParameter('page') : 0;
		$pcnt = FALSE;

		if($request !== FALSE && $request->getId() !== FALSE)
			return $this->display($engine, $request);
		$page = new Page;
		$page->append('title', array('stock' => $this->name,
				'text' => $this->content_title));
		//obtain the total number of records available
		if(($res = $db->query($engine, $this->query_list_count.$query,
				array('module_id' => $this->id))) !== FALSE
				&& count($res) == 1)
			$pcnt = $res[0][0];
		if(is_string(($order = $this->content_list_order)))
			$query .= ' ORDER BY '.$order;
		if(($limit = $this->content_list_count) > 0 && is_int($limit))
			$query .= ' LIMIT '.$limit;
		if(is_numeric($p) && $p > 1)
			$query .= ' OFFSET '.($limit * ($p - 1));
		if(($res = $db->query($engine, $this->query_list.$query,
				array('module_id' => $this->id))) === FALSE)
		{
			$error = _('Unable to list contents');
			$page->append('dialog', array('type' => 'error',
						'text' => $error));
			return $page;
		}
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
			$page->appendElement($this->preview($engine,
						$res[$i]['id']));
		//output paging information
		if($pcnt !== FALSE && ($pcnt > $this->content_list_count))
		{
			//FIXME implement
		}
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


	//ContentModule::display
	protected function display($engine, $request)
	{
		$error = _('Could not display content');

		if(($id = $request->getId()) === FALSE)
			return $this->_default($engine, $request);
		if(($page = $this->_display($engine, $id, $request->getTitle()))
				=== FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'error' => $error));
		return $page;
	}


	//ContentModule::_display
	protected function _display($engine, $id, $title = FALSE)
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
		$page->append('label', array('text' => $content['content']
			."\n"));
		$r = new Request($engine, $this->name);
		$page->append('link', array('stock' => 'back', 'request' => $r,
				'text' => $this->content_more_content));
		return $page;
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


	//ContentModule::headline
	protected function headline($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();

		$columns = array('title' => _('Title'));
		$view = new PageElement('treeview', array('view' => 'details',
				'columns' => $columns));
		$query = $this->query_list;
		$query .= ' ORDER BY timestamp DESC LIMIT 6'; //XXX define limit
		if(($res = $db->query($engine, $query, array(
					'module_id' => $this->id))) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Unable to list contents')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $view->append('row');
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
				'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
		}
		return $view;
	}


	//ContentModule::list
	protected function _list($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$uid = ($request !== FALSE) ? $request->getId() : FALSE;

		$page = new Page;
		$title = $this->content_list_title;
		if($uid !== FALSE)
			$title = $this->content_list_title_by.' '.$uid; //XXX
		$page->setProperty('title', $title);
		$element = $page->append('title', array(
				'stock' => $this->name));
		$query = ($uid !== FALSE) ? $this->query_list_user
			: $this->query_list;
		$query .= ' ORDER BY title ASC';
		if(($res = $db->query($engine, $query, array(
				'module_id' => $this->id, 'user_id' => $uid)))
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
			$r = new Request($engine, $this->name, FALSE,
				$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
				'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$date = $this->_timestampToDate(_('d/m/Y H:i'),
					$res[$i]['timestamp']);
			$row->setProperty('date', $date);
		}
		$r = new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back')));
		return $page;
	}


	//ContentModule::preview
	protected function preview($engine, $id, $title = FALSE)
	{
		$error = _('Could not fetch content');

		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return new PageElement('dialog', array(
						'type' => 'error',
						'text' => $error));
		return $this->_preview($engine, $content);
	}

	protected function _preview($engine, $content, $preview = FALSE)
	{
		$page = new PageElement('vbox');
		$r = new Request($engine, $content['module'], FALSE,
			$content['id'], $content['title']);
		$title = $page->append('title');
		if($preview !== FALSE)
		{
			$title->setProperty('stock', 'preview');
			$title->setProperty('text', $content['title']);
		}
		else
			$title->append('link', array('request' => $r,
					'text' => $content['title']));
		$page->append('label', array('text' => $this->content_by.' '
					.$content['username']
					._(' on ').$content['date']));
		$hbox = $page->append('hbox');
		$hbox->append('image', array('stock' => 'module '
					.$content['module'].' content'));
		$text = $content['content'];
		if($this->content_preview_length > 0
			&& strlen($text) > $this->content_preview_length)
			$text = substr($text, 0,
				$this->content_preview_length).'...';
		$hbox->append('label', array('text' => $text));
		if($preview === FALSE)
			$page->append('button', array(
					'stock' => $this->content_open_stock,
					'text' => $this->content_open_text,
					'request' => $r));
		return $page;
	}


	//ContentModule::submit
	protected function submit($engine, $request = FALSE)
	{
		$title = $this->content_submit;
		$error = _('Permission denied');

		if(!$this->canSubmit($engine, $request, $error))
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//FIXME really implement
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title));
		return $page;
	}


	//ContentModule::update
	protected function update($engine, $request)
	{
		$error = _('Unable to fetch content');

		if(($id = $request->getId()) === FALSE
				|| ($content = $this->_get($engine, $id))
				=== FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		$title = _('Update ').$content['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title));
		if($request->getParameter('preview') !== FALSE)
		{
			$preview = array('id' => $id,
				'module' => $this->name,
				'username' => $content['username'],
				'date' => $content['date'],
				'title' => _('Preview: ').$request->getTitle(),
				'content' => $request->getParameter('content'));
			$page->appendElement($this->_preview($engine,
					$preview, TRUE));
		}
		//FIXME really implement
		$r = new Request($engine, $this->name, 'update', $id);
		$form = $page->append('form', array('request' => $r));
		$vbox = $form->append('vbox');
		if(($value = $request->getTitle()) === FALSE)
			$value = $content['title'];
		$vbox->append('entry', array('name' => 'title',
				'text' => _('Title: '), 'value' => $value));
		$vbox->append('label', array('text' => _('Content: ')));
		if(($value = $request->getParameter('content')) === FALSE)
			$value = $content['content'];
		$vbox->append('textview', array('name' => 'content',
				'value' => $value));
		$hbox = $vbox->append('hbox');
		$r = new Request($engine, $this->name, FALSE, $request->getId(),
				$content['title']);
		$hbox->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		$hbox->append('button', array('type' => 'reset',
				'stock' => 'reset', 'text' => _('Reset')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'preview', 'name' => 'preview',
				'text' => _('Preview')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'submit', 'text' => _('Submit')));
		return $page;
	}
}

?>
