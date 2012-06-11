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
//- let users save and publish their contents themselves
//- complete paging



require_once('./system/content.php');
require_once('./system/module.php');


//ContentModule
class ContentModule extends Module
{
	//public
	//methods
	//essential
	//ContentModule::ContentModule
	public function __construct($id, $name, $title = FALSE)
	{
		parent::__construct($id, $name, $title);
		//translations
		$this->text_content_admin = _('Content administration');
		$this->text_content_by = _('Content by');
		$this->text_content_item = _('Content');
		$this->text_content_items = _('Content');
		$this->text_content_list_title = _('Content list');
		$this->text_content_list_title_by = _('Content by');
		$this->text_content_more_content = _('More content...');
		$this->text_content_on = _('on');
		$this->text_content_open = _('Read');
		$this->text_content_publish = _('Publish');
		$this->text_content_submit = _('Submit content');
		$this->text_content_submit_progress
			= _('Submission in progress, please wait...');
		$this->text_content_title = _('Content');
		$this->text_content_update = _('Update');
	}


	//useful
	//ContentModule::call
	public function call(&$engine, $request)
	{
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'admin':
			case 'default':
			case 'delete':
			case 'disable':
			case 'display':
			case 'enable':
			case 'headline':
			case 'list':
			case 'preview':
			case 'submit':
			case 'update':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return FALSE;
	}


	//protected
	//properties
	protected $content_headline_count = 6;
	protected $content_list_count = 10;
	protected $content_list_order = 'timestamp DESC';
	protected $content_open_stock = 'read';
	protected $content_preview_length = 150;
	protected $text_content_admin = 'Content administration';
	protected $text_content_by = 'Content by';
	protected $text_content_item = 'Content';
	protected $text_content_items = 'Content';
	protected $text_content_list_title = 'Content list';
	protected $text_content_list_title_by = 'Content by';
	protected $text_content_more_content = 'More content...';
	protected $text_content_on = 'on';
	protected $text_content_open = 'Read';
	protected $text_content_publish = 'Publish';
	protected $text_content_submit = 'Submit content';
	protected $text_content_submit_progress
		= 'Submission in progress, please wait...';
	protected $text_content_title = 'Content';
	protected $text_content_update = 'Update';

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
		daportal_user.user_id AS user_id,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content, timestamp,
		daportal_content.enabled AS enabled, public
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND (daportal_content.public='1' OR daportal_content.user_id=:user_id)
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_content.content_id=:content_id";
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
		AND daportal_user.enabled='1'
		ORDER BY timestamp DESC";
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
	//ContentModule::canPreview
	protected function canPreview($engine, $request = FALSE)
	{
		return TRUE;
	}


	//ContentModule::canPublish
	protected function canPublish($engine, $id, &$error = FALSE)
	{
		global $config;
		$cred = $engine->getCredentials();

		$error = _('Permission denied');
		if($cred->getUserId() == 0)
			return FALSE;
		if($cred->isAdmin())
			return TRUE;
		$content = Content::get($engine, $this->id, $id);
		if($id !== $content['id'])
			return FALSE;
		$moderate = $config->getVariable('module::'.$this->name,
				'moderate');
		return ($moderate === FALSE || $moderate == 0) ? TRUE : FALSE;
	}


	//ContentModule::canSubmit
	protected function canSubmit($engine, &$error = FALSE)
	{
		global $config;
		$cred = $engine->getCredentials();

		if($cred->getUserId() > 0)
			return TRUE;
		if($config->getVariable('module::'.$this->name, 'anonymous'))
			return TRUE;
		$error = _('Permission denied');
		return FALSE;
	}


	//ContentModule::canUpdate
	protected function canUpdate($engine, &$error = FALSE)
	{
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			return TRUE;
		//FIXME really implement
		$error = _('Permission denied');
		return FALSE;
	}


	//ContentModule::_get
	protected function _get($engine, $id, $title = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$query = $this->query_get;

		if($id === FALSE)
			return FALSE;
		$args = array('module_id' => $this->id, 'content_id' => $id,
				'user_id' => $cred->getUserId());
		if($title !== FALSE)
		{
			$query .= ' AND daportal_content.title LIKE :title';
			$args['title'] = str_replace('-', '_', $title);
		}
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return FALSE;
		$res = $res[0];
		$res['date'] = $this->_timestampToDate($res['timestamp'],
				_('d/m/Y H:i:s'));
		return $res;
	}


	//ContentModule::getToolbar
	protected function _getToolbar($engine, $id = FALSE, $title = FALSE)
	{
		$cred = $engine->getCredentials();

		$toolbar = new PageElement('toolbar');
		if($cred->isAdmin($engine))
		{
			$r = new Request($engine, $this->name, 'admin');
			$toolbar->append('button', array('request' => $r,
				'stock' => 'admin',
				'text' => _('Administration')));
		}
		if($this->canSubmit($engine))
		{
			$r = new Request($engine, $this->name, 'submit');
			$toolbar->append('button', array('request' => $r,
				'stock' => 'new',
				'text' => $this->text_content_submit));
		}
		if($id !== FALSE && ($content = $this->_get($engine, $id,
				$title)) !== FALSE)
		{
			if($content['public'] == FALSE
					&& $this->canPublish($engine))
			{
				$r = new Request($engine, $this->name,
					'publish', $id, $content['title']);
				$toolbar->append('button', array(
					'request' => $r,
					'stock' => 'publish',
					'text' => $this->text_content_publish));
			}
			if($this->canUpdate($engine, $id))
			{
				$r = new Request($engine, $this->name,
						'update', $content['id'],
						$content['title']);
				$toolbar->append('button', array(
					'request' => $r,
					'stock' => 'update',
					'text' => $this->text_content_update));
			}
		}
		return $toolbar;
	}


	//convertors
	//ContentModule::_timestampToDate
	//FIXME move to the SQL module?
	protected function _timestampToDate($timestamp, $format = 'd/m/Y H:i:s')
	{
		$date = substr($timestamp, 0, 19);
		$date = strtotime($date);
		//FIXME use strftime() instead
		$date = date($format, $date);
		return $date;
	}


	//forms
	//ContentModule::formSubmit
	protected function formSubmit($engine, $request)
	{
		$r = new Request($engine, $this->name, 'submit');
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		$vbox->append('entry', array('name' => 'title',
				'text' => _('Title: '),
				'value' => $request->getTitle()));
		$vbox->append('textview', array('name' => 'content',
				'text' => _('Content: '),
				'value' => $request->getParameter('content')));
		$r = new Request($engine, $this->name);
		$form->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		if($this->canPreview($engine, $request))
			$form->append('button', array('type' => 'submit',
					'stock' => 'preview',
					'name' => 'action',
					'value' => 'preview',
					'text' => _('Preview')));
		$form->append('button', array('type' => 'submit',
				'stock' => 'submit', 'name' => 'action',
				'value' => 'submit', 'text' => _('Submit')));
		return $form;
	}


	//actions
	//ContentModule::actions
	protected function actions($engine, $request)
	{
		$cred = $engine->getCredentials();

		$ret = array();
		if($cred->isAdmin())
		{
			$ret[] = array();
			$icon = new PageElement('image', array(
				'stock' => 'admin'));
			$r = new Request($engine, $this->name, 'admin');
			$link = new PageElement('link', array('request' => $r,
				'text' => _('Administration')));
			$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		}
		if($this->canSubmit($engine))
		{
			$icon = new PageElement('image', array(
				'stock' => 'new'));
			$r = new Request($engine, $this->name, 'submit');
			$link = new PageElement('link', array('request' => $r,
				'text' => $this->text_content_submit));
			$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		}
		return $ret;
	}


	//calls
	//ContentModule::callAdmin
	protected function callAdmin($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$database = $engine->getDatabase();
		$actions = array('delete', 'disable', 'enable');

		//check credentials
		if(!$cred->isAdmin())
		{
			$error = _('Permission denied');
			$r = new Request($engine, 'user', 'login');
			$dialog = new PageElement('dialog', array(
						'type' => 'error',
						'text' => $error));
			$dialog->append('button', array('stock' => 'login',
						'text' => _('Login'),
						'request' => $r));
			return $dialog;
		}
		//perform actions if necessary
		if($request !== FALSE)
			foreach($actions as $a)
				if($request->getParameter($a) !== FALSE)
					return $this->$a($engine, $request);
		//administrative page
		$page = new Page;
		$title = $this->text_content_admin;
		$page->setProperty('title', $title);
		$element = $page->append('title', array('stock' => 'admin',
				'text' => $title));
		$query = $this->query_list_admin;
		if(($res = $database->query($engine, $query, array(
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
		$toolbar->append('button', array('stock' => 'delete',
					'text' => _('Delete'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'delete'));
		$no = new PageElement('image', array('stock' => 'no',
				'size' => 16, 'title' => _('Disabled')));
		$yes = new PageElement('image', array('stock' => 'yes',
				'size' => 16, 'title' => _('Enabled')));
		//FIXME add controls for publication
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('id', 'content_id:'.$res[$i]['id']);
			$r = new Request($engine, $this->name, 'update',
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$row->setProperty('enabled', $database->isTrue(
					$res[$i]['enabled']) ? $yes : $no);
			$r = new Request($engine, 'user', FALSE,
					$res[$i]['user_id'],
					$res[$i]['username']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $res[$i]['username']));
			$row->setProperty('username', $link);
			$date = $this->_timestampToDate(_('d/m/Y H:i:s'),
					$res[$i]['timestamp']);
			$row->setProperty('date', $date);
		}
		$vbox = $page->append('vbox');
		$r = new Request($engine, $this->name);
		$vbox->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back to this module')));
		$r = new Request($engine, 'admin');
		$vbox->append('link', array('request' => $r, 'stock' => 'admin',
				'text' => _('Back to the administration')));
		return $page;
	}


	//ContentModule::callDefault
	protected function callDefault($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$query = '';
		$p = ($request !== FALSE) ? $request->getParameter('page') : 0;
		$pcnt = FALSE;

		if($request !== FALSE && $request->getId() !== FALSE)
			return $this->actionDisplay($engine, $request);
		$page = new Page(array('title' => $this->text_content_title));
		$page->append('title', array('stock' => $this->name,
				'text' => $this->text_content_title));
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
			$page->append($this->preview($engine, $res[$i]['id']));
		//output paging information
		if($pcnt !== FALSE && ($pcnt > $this->content_list_count))
		{
			//FIXME implement
		}
		return $page;
	}


	//ContentModule::callDelete
	protected function callDelete($engine, $request)
	{
		$query = $this->query_delete;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_delete;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be deleted successfully'),
				_('Some content could not be deleted'));
	}


	//ContentModule::callDisable
	protected function callDisable($engine, $request)
	{
		$query = $this->query_disable;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_disable;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be disabled successfully'),
				_('Some content could not be disabled'));
	}


	//ContentModule::callDisplay
	protected function callDisplay($engine, $request)
	{
		$error = _('Could not display content');

		if(($id = $request->getId()) === FALSE)
			return $this->_default($engine, $request);
		if(($content = $this->_get($engine, $id, $request->getTitle()))
				=== FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'error' => $error));
		//page
		$page = new Page(array('title' => $content['title']));
		//toolbar
		$toolbar = $this->_getToolbar($engine, $request);
		$this->_display($engine, $page, $toolbar, $content);
		//bottom
		$r = new Request($engine, $this->name);
		$page->append('link', array('stock' => 'back', 'request' => $r,
				'text' => $this->text_content_more_content));
		return $page;
	}


	//ContentModule::_display
	protected function _display($engine, $page, $toolbar, $content)
	{
		$title = new PageElement('title', array('stock' => $this->name,
			'text' => $content['title']));
		$page->append($title);
		if($toolbar !== FALSE)
			$page->append($toolbar);
		$content = new PageElement('label', array(
			'text' => $content['content']."\n"));
		$page->append($content);
		//FIXME display metadata and link to actual resource?
		return $page;
	}


	//ContentModule::callEnable
	protected function callEnable($engine, $request)
	{
		$query = $this->query_enable;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_enable;
		return $this->_apply($engine, $request, $query, 'admin',
				_('Content could be enabled successfully'),
				_('Some content could not be enabled'));
	}


	//ContentModule::callHeadline
	protected function callHeadline($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();

		$columns = array('title' => _('Title'));
		$view = new PageElement('treeview', array('view' => 'details',
				'columns' => $columns));
		$query = $this->query_list;
		$count = (is_integer($this->content_headline_count))
			? $this->content_headline_count : 6;
		$query .= ' ORDER BY timestamp DESC LIMIT '.$count;
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


	//ContentModule::callList
	protected function callList($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$uid = ($request !== FALSE) ? $request->getId() : FALSE;

		$title = $this->text_content_list_title;
		if($uid !== FALSE)
			//XXX get the proper title
			$title = $this->text_content_list_title_by.' '.$uid;
		$page = new Page(array('title' => $title));
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
			$date = $this->_timestampToDate(_('d/m/Y H:i:s'),
					$res[$i]['timestamp']);
			$row->setProperty('date', $date);
		}
		$r = new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back')));
		return $page;
	}


	//ContentModule::callPreview
	protected function callPreview($engine, $request)
	{
		$error = _('Could not fetch content');

		//obtain the content
		if(($content = $this->_get($engine, $request->getId(),
				$request->getTitle())) === FALSE)
			return new PageElement('dialog', array(
						'type' => 'error',
						'text' => $error));
		$preview = new PageElement('vbox');
		//link
		$r = new Request($engine, $content['module'], FALSE,
				$content['id'], $content['title']);
		//title
		$this->helperPreviewTitle($engine, $preview, $r, $content);
		//meta-data
		$this->helperPreviewMetadata($engine, $preview, $r, $content);
		//text
		$this->helperPreviewText($engine, $preview, $r, $content);
		//buttons
		$this->helperPreviewButtons($engine, $preview, $r, $content);
		return $preview;
	}


	//ContentModule::callSubmit
	protected function callSubmit($engine, $request = FALSE)
	{
		$title = $this->text_content_submit;
		$error = _('Permission denied');

		//check permissions
		if(!$this->canSubmit($engine, $error))
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//create the page
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//toolbar
		$toolbar = $this->_getToolbar($engine, $request);
		$page->append($toolbar);
		//process the content
		$content = FALSE;
		if(($error = $this->_submitProcess($engine, $request, $content))
				=== FALSE)
			return $this->_submitSuccess($engine, $request, $page,
					$content);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		//preview
		if($request->getParameter('preview') !== FALSE)
		{
			$content = array('title' => _('Preview: ')
					.$request->getTitle(),
				'content' => $request->getParameter('content'));
			$this->_display($engine, $page, $toolbar, $content);
		}
		$form = $this->formSubmit($engine, $request);
		$page->append($form);
		return $page;
	}

	protected function _submitProcess($engine, $request, &$content)
	{
		//verify the request
		if($request->getParameter('submit') === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		//store the content uploaded
		$title = $request->getTitle();
		$content = $request->getParameter('content');
		$public = $request->getParameter('public') ? TRUE : FALSE;
		$content = Content::insert($engine, $this->id, $title, $content,
			FALSE, TRUE);
		if($content === FALSE)
			return _('Internal server error');
		return FALSE;
	}

	protected function _submitSuccess($engine, $request, $page, $content)
	{
		$r = new Request($engine, $this->name, FALSE, $content->getId(),
				$content->getTitle());
		$page->setProperty('location', $engine->getUrl($r));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$text = $this->text_content_submit_progress;
		$box->append('label', array('text' => $text));
		$box = $box->append('hbox');
		$text = _('If you are not redirected within 30 seconds, please ');
		$box->append('label', array('text' => $text));
		$box->append('link', array('text' => _('click here'),
				'request' => $r));
		$box->append('label', array('text' => '.'));
		return $page;
	}


	//ContentModule::callUpdate
	protected function callUpdate($engine, $request)
	{
		$error = _('Unable to fetch content');

		if(($id = $request->getId()) === FALSE
				|| ($content = $this->_get($engine, $id))
				=== FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		$title = _('Update ').$content['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		if($request->getParameter('preview') !== FALSE)
		{
			$preview = array('id' => $id, 'module' => $this->name,
				'username' => $content['username'],
				'date' => $content['date'],
				'title' => _('Preview: ').$request->getTitle(),
				'content' => $request->getParameter('content'));
			$page->append($this->_preview($engine, $preview, TRUE));
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
				'stock' => 'preview', 'name' => 'action',
				'value' => 'preview', 'text' => _('Preview')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'submit', 'name' => 'action',
				'value' => 'submit', 'text' => _('Submit')));
		return $page;
	}


	//helpers
	//ContentModule::helperApply
	protected function helperApply($engine, $request, $query, $fallback,
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
		if($request->isIdempotent())
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


	//ContentModule::helperPreviewButtons
	protected function helperPreviewButtons($engine, $preview, $request,
			$content)
	{
		$preview->append('button', array('request' => $request,
				'stock' => $this->content_open_stock,
				'text' => $this->text_content_open));
	}


	//ContentModule::helperPreviewMetadata
	protected function helperPreviewMetadata($engine, $preview, $request,
			$content)
	{
		$r = new Request($engine, 'user', FALSE,
				$content['user_id'], $content['username']);
		$link = new PageElement('link', array('request' => $r,
				'text' => $content['username']));
		$meta = $preview->append('label', array(
				'text' => $this->text_content_by.' '));
		$meta->append($link);
		$meta = $meta->append('label', array(
				'text' => ' '.$this->text_content_on.' '
				.$content['date']));
	}


	//ContentModule::helperPreviewText
	protected function helperPreviewText($engine, $preview, $request,
			$content)
	{
		$text = $content['content'];
		if($this->content_preview_length > 0
				&& strlen($text)
				> $this->content_preview_length)
			//abbreviate text as required
			$text = substr($text, 0,
				$this->content_preview_length).'...';
		$preview->append('label', array('text' => $text));
	}


	//ContentModule::helperPreviewTitle
	protected function helperPreviewTitle($engine, $preview, $request,
			$content)
	{
		$link = new PageElement('link', array('request' => $request,
				'text' => $content['title']));
		$title = $preview->append('title');
		$title->append($link);
	}
}

?>
