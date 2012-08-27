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
//- list contents pending moderation (if relevant)



require_once('./system/content.php');
require_once('./system/module.php');


//ContentModule
abstract class ContentModule extends Module
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
		$this->text_content_headline_title = _('Content headlines');
		$this->text_content_item = _('Content');
		$this->text_content_items = _('Content');
		$this->text_content_link = _('Permalink');
		$this->text_content_list_title = _('Content list');
		$this->text_content_list_title_by = _('Content by');
		$this->text_content_more_content = _('More content...');
		$this->text_content_on = _('on');
		$this->text_content_open = _('Read');
		$this->text_content_post = _('Publish');
		$this->text_content_publish_progress
			= _('Publication in progress, please wait...');
		$this->text_content_submit = _('Submit content');
		$this->text_content_submit_progress
			= _('Submission in progress, please wait...');
		$this->text_content_title = _('Content');
		$this->text_content_update = _('Update');
		$this->text_content_update_progress
			= _('Update in progress, please wait...');
	}


	//useful
	//ContentModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'actions':
				return $this->$action($engine, $request);
			case 'admin':
			case 'default':
			case 'display':
			case 'headline':
			case 'list':
			case 'preview':
			case 'publish':
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
	protected $text_content_headline_title = 'Content headlines';
	protected $text_content_item = 'Content';
	protected $text_content_items = 'Content';
	protected $text_content_link = 'Permalink';
	protected $text_content_list_title = 'Content list';
	protected $text_content_list_title_by = 'Content by';
	protected $text_content_more_content = 'More content...';
	protected $text_content_on = 'on';
	protected $text_content_open = 'Read';
	protected $text_content_post = 'Publish';
	protected $text_content_publish_progress
		= 'Publication in progress, please wait...';
	protected $text_content_submit = 'Submit content';
	protected $text_content_submit_progress
		= 'Submission in progress, please wait...';
	protected $text_content_title = 'Content';
	protected $text_content_update = 'Update';
	protected $text_content_update_progress
			= 'Update in progress, please wait...';

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
	protected $query_admin_post = "UPDATE daportal_content
		SET public='1'
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
		title, daportal_content.enabled AS enabled, content
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
		daportal_group.group_id AS group_id, groupname,
		title, daportal_content.enabled AS enabled,
		daportal_content.public AS public
		FROM daportal_content, daportal_module, daportal_user,
		daportal_group
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.group_id=daportal_group.group_id
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
	protected $query_post = "UPDATE daportal_content
		SET public='1'
		WHERE module_id=:module_id
		AND content_id=:content_id AND user_id=:user_id";


	//methods
	//accessors
	//ContentModule::canAdmin
	protected function canAdmin($engine, $content = FALSE, &$error = FALSE)
	{
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			return TRUE;
		$error = _('Permission denied');
		return FALSE;
	}


	//ContentModule::canPreview
	protected function canPreview($engine, $request = FALSE)
	{
		return TRUE;
	}


	//ContentModule::canPost
	protected function canPost($engine, $content, &$error = FALSE)
	{
		global $config;
		$cred = $engine->getCredentials();

		$error = _('Permission denied');
		if($cred->getUserId() == 0)
			return FALSE;
		if($cred->isAdmin())
			return TRUE;
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
	protected function canUpdate($engine, $content = FALSE, &$error = FALSE)
	{
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			return TRUE;
		//FIXME really implement
		$error = _('Permission denied');
		return FALSE;
	}


	//ContentModule::_get
	//XXX remove $id and $title?
	protected function _get($engine, $id, $title = FALSE, $request = FALSE)
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
		$res['date'] = $db->formatDate($engine, $res['timestamp']);
		return $res;
	}


	//ContentModule::getToolbar
	protected function getToolbar($engine, $content = FALSE)
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
		if($content !== FALSE)
		{
			if($content['public'] == FALSE
					&& $this->canPost($engine, $content))
			{
				$r = new Request($engine, $this->name,
					'publish', $content['id'],
					$content['title']);
				$toolbar->append('button', array(
					'request' => $r,
					'stock' => 'post',
					'text' => $this->text_content_post));
			}
			if($this->canUpdate($engine, $content))
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
	//ContentModule::timestampToDate
	protected function timestampToDate($timestamp = FALSE,
			$format = '%d/%m/%Y %H:%M:%S')
	{
		if($timestamp === FALSE)
			$timestamp = time();
		$date = strftime($format, $timestamp);
		return $date;
	}


	//forms
	//ContentModule::formSubmit
	protected function formSubmit($engine, $request)
	{
		$r = new Request($engine, $this->name, 'submit');
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		//title
		$this->helperSubmitTitle($engine, $request, $vbox);
		//content
		$this->helperSubmitContent($engine, $request, $vbox);
		//buttons
		$this->helperSubmitButtons($engine, $request, $form);
		return $form;
	}


	//ContentModule::formUpdate
	protected function formUpdate($engine, $request, $content)
	{
		$r = new Request($engine, $this->name, 'update',
				$content['id']);
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		//title
		$this->helperUpdateTitle($engine, $request, $vbox, $content);
		//content
		$this->helperUpdateContent($engine, $request, $vbox, $content);
		//buttons
		$this->helperUpdateButtons($engine, $request, $vbox, $content);
		return $form;
	}


	//actions
	//ContentModule::actions
	protected function actions($engine, $request)
	{
		$cred = $engine->getCredentials();

		if(($user = $request->getParameter('user')) !== FALSE)
			return $this->helperActionsUser($engine, $request,
					$user);
		$ret = array();
		if($cred->isAdmin())
		{
			$r = $this->helperActionsAdmin($engine, $request);
			$ret = array_merge($ret, $r);
		}
		if($request->getParameter('admin') != 0)
			return $ret;
		if($this->canSubmit($engine))
		{
			$r = $this->helperActionsSubmit($engine, $request);
			$ret = array_merge($ret, $r);
		}
		if(($r = $this->helperActions($engine, $request)) !== FALSE)
			$ret = array_merge($ret, $r);
		return $ret;
	}


	//calls
	//ContentModule::callAdmin
	protected function callAdmin($engine, $request = FALSE)
	{
		$database = $engine->getDatabase();
		$query = $this->query_list_admin;
		$actions = array('delete', 'disable', 'enable', 'post');
		$error = FALSE;

		//check credentials
		if(!$this->canAdmin($engine, FALSE, $error))
		{
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
				{
					$a = 'call'.ucfirst($a);
					return $this->$a($engine, $request);
				}
		//administrative page
		$page = new Page;
		$title = $this->text_content_admin;
		$page->setProperty('title', $title);
		$element = $page->append('title', array('stock' => 'admin',
				'text' => $title));
		$args = array('module_id' => $this->id);
		$error = _('Unable to list contents');
		if(($res = $database->query($engine, $query, $args)) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		$r = new Request($engine, $this->name, 'admin');
		if($request !== FALSE && ($type = $request->getParameter(
					'type')) !== FALSE)
			$r->setParameter('type', $type);
		$treeview = $page->append('treeview', array('request' => $r));
		$treeview->setProperty('columns', array('icon' => '',
				'title' => _('Title'),
				'enabled' => _('Enabled'),
				'public' => _('Public'),
				'username' => _('Username'),
				'date' => _('Date')));
		//toolbar
		$this->helperAdminToolbar($engine, $treeview, $request);
		//rows
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$this->helperAdminRow($engine, $row, $res[$i]);
		}
		//buttons
		$vbox = $page->append('vbox');
		$this->helperAdminButtons($engine, $vbox, $request);
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
			return $this->callDisplay($engine, $request);
		$page = new Page(array('title' => $this->text_content_title));
		$page->append('title', array('stock' => $this->name,
				'text' => $this->text_content_title));
		//obtain the total number of records available
		if(($res = $db->query($engine, $this->query_list_count,
				array('module_id' => $this->id))) !== FALSE
				&& count($res) == 1)
			$pcnt = $res[0][0];
		if(is_string(($order = $this->content_list_order)))
			$query .= ' ORDER BY '.$order;
		//paging
		$limit = FALSE;
		if($this->content_list_count > 0)
		{
			$limit = $this->content_list_count;
			$offset = FALSE;
			if(is_numeric($p) && $p > 1)
				$offset = $limit * ($p - 1);
			$query .= $db->offset($limit, $offset);
		}
		//query
		if(($res = $db->query($engine, $this->query_list.$query,
				array('module_id' => $this->id))) === FALSE)
		{
			$error = _('Unable to list contents');
			$page->append('dialog', array('type' => 'error',
						'text' => $error));
			return $page;
		}
		$vbox = $this->helperPreviewHeader($engine, $request, $page);
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$content = $this->_get($engine, $res[$i]['id']);
			$this->helperPreview($engine, $vbox, $content);
		}
		//output paging information
		$this->helperPaging($engine, $request, $page, $pcnt);
		return $page;
	}


	//ContentModule::callDelete
	protected function callDelete($engine, $request)
	{
		$query = $this->query_delete;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_delete;
		return $this->helperApply($engine, $request, $query, 'admin',
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
		return $this->helperApply($engine, $request, $query, 'admin',
				_('Content could be disabled successfully'),
				_('Some content could not be disabled'));
	}


	//ContentModule::callDisplay
	protected function callDisplay($engine, $request)
	{
		$error = _('Could not display content');

		//obtain the content
		if(($id = $request->getId()) === FALSE)
			return $this->callDefault($engine, $request);
		if(($content = $this->_get($engine, $id, $request->getTitle(),
				$request)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		$page = new Page(array('title' => $content['title']));
		$this->helperDisplay($engine, $page, $content);
		return $page;
	}


	//ContentModule::callEnable
	protected function callEnable($engine, $request)
	{
		$query = $this->query_enable;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_enable;
		return $this->helperApply($engine, $request, $query, 'admin',
				_('Content could be enabled successfully'),
				_('Some content could not be enabled'));
	}


	//ContentModule::callHeadline
	protected function callHeadline($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $this->query_list;
		$title = $this->text_content_headline_title;

		//view
		$columns = array('title' => _('Title'), 'date' => _('Date'),
				'username' => _('Author'));
		$view = new PageElement('treeview', array('view' => 'details',
				'title' => $title, 'columns' => $columns));
		//obtain contents
		$count = (is_integer($this->content_headline_count))
			? $this->content_headline_count : 6;
		$query .= ' ORDER BY timestamp DESC LIMIT '.$count;
		$error = _('Unable to list contents');
		if(($res = $db->query($engine, $query, array(
					'module_id' => $this->id))) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		//rows
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $view->append('row');
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
				'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$row->setProperty('timestamp', $res[$i]['timestamp']);
			$row->setProperty('date', $db->formatDate($engine,
					$res[$i]['timestamp']));
			$r = new Request($engine, 'user', FALSE,
					$res[$i]['user_id'],
					$res[$i]['username']);
			$link = new PageElement('link', array('request' => $r,
					'stock' => 'user',
					'text' => $res[$i]['username']));
			$row->setProperty('username', $link);
			//XXX write a helper for this
			$content = $res[$i]['content'];
			if(($len = $this->content_preview_length) > 0
					&& strlen($content) > $len)
				$content = substr($content, 0, $len).'...';
			$row->setProperty('content', $content);
		}
		return $view;
	}


	//ContentModule::callList
	protected function callList($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$user = new User($engine, $request->getId(),
				$request->getTitle());
		$error = _('Unable to list contents');

		if(($uid = $user->getUserId()) == 0)
			$uid = FALSE;
		$title = $this->text_content_list_title;
		if($uid !== FALSE)
			$title = $this->text_content_list_title_by.' '
				.$user->getUsername();
		//title
		$page = new Page(array('title' => $title));
		$this->helperListTitle($engine, $page, $request);
		//query
		$args = array('module_id' => $this->id);
		$query = $this->query_list;
		if($uid !== FALSE)
		{
			$query = $this->query_list_user;
			$args['user_id'] = $uid;
		}
		$query .= ' ORDER BY title ASC';
		if(($res = $db->query($engine, $query, $args)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//view
		$r = FALSE;
		if($uid === $cred->getUserId())
			$r = new Request($engine, $this->name, 'list', $uid,
				$uid ? $user->getUsername() : FALSE);
		$treeview = $page->append('treeview', array('request' => $r));
		$columns = array('title' => _('Title'));
		if($uid === $cred->getUserId())
			$columns['enabled'] = _('Enabled');
		$columns['username'] = _('Username');
		$columns['date'] = _('Date');
		$treeview->setProperty('columns', $columns);
		//toolbar
		$this->helperListToolbar($engine, $treeview, $request);
		//rows
		$no = new PageElement('image', array('stock' => 'no',
			'size' => 16, 'title' => _('Disabled')));
		$yes = new PageElement('image', array('stock' => 'yes',
			'size' => 16, 'title' => _('Enabled')));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('id', 'content_id:'.$res[$i]['id']);
			//title
			$r = new Request($engine, $this->name, FALSE,
				$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('request' => $r,
				'text' => $res[$i]['title']));
			$row->setProperty('title', $link);
			$row->setProperty('enabled', $db->isTrue(
					$res[$i]['enabled']) ? $yes : $no);
			//username
			$r = new Request($engine, 'user', FALSE,
					$res[$i]['user_id'],
					$res[$i]['username']);
			$link = new PageElement('link', array('request' => $r,
					'stock' => 'user',
					'text' => $res[$i]['username']));
			$row->setProperty('username', $link);
			$date = $db->formatDate($engine, $res[$i]['timestamp']);
			$row->setProperty('date', $date);
			//XXX hack for ProjectModule
			if(isset($res[$i]['synopsis']))
				$row->setProperty('synopsis',
						$res[$i]['synopsis']);
		}
		//buttons
		$this->helperListButtons($engine, $page, $request);
		return $page;
	}


	//ContentModule::callPost
	protected function callPost($engine, $request)
	{
		$query = $this->query_post;
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			$query = $this->query_admin_post;
		return $this->helperApply($engine, $request, $query, 'admin',
				_('Content could be posted successfully'),
				_('Some content could not be posted'));
	}


	//ContentModule::callPreview
	protected function callPreview($engine, $request)
	{
		$error = _('Could not preview content');

		//obtain the content
		if(($content = $this->_get($engine, $request->getId(),
				$request->getTitle(), $request)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		$page = new Page(array('title' => $content['title']));
		$this->helperPreview($engine, $page, $content);
		return $page;
	}


	//ContentModule::callPublish
	protected function callPublish($engine, $request)
	{
		$error = _('Could not preview content');

		//obtain the content
		if(($content = $this->_get($engine, $request->getId(),
				$request->getTitle(), $request)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//FIXME make this a dialog instead
		//check permissions
		if($this->canPost($engine, $content, $error) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//create the page
		$title = $this->text_content_post.' '.$content['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
			'text' => $title));
		//toolbar
		$toolbar = $this->getToolbar($engine);
		$page->append($toolbar);
		//process the request
		if(($error = $this->_publishProcess($engine, $request,
				$content)) === FALSE)
			return $this->_publishSuccess($engine, $request, $page,
					$content);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		//preview
		$vbox = $page->append('vbox');
		unset($content['id']);
		$content['title'] = _('Preview: ').$content['title'];
		$this->helperPreview($engine, $vbox, $content);
		//form
		$r = new Request($engine, $this->name, 'publish',
				$request->getId(), $request->getTitle());
		$form = $page->append('form', array('request' => $r));
		//buttons
		$r = new Request($engine, $this->name, FALSE, $request->getId(),
				$request->getTitle());
		$form->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		$form->append('button', array('type' => 'submit',
				'name' => 'action', 'value' => 'publish',
				'text' => $this->text_content_post));
		return $page;
	}

	protected function _publishProcess($engine, $request, $content)
	{
		$db = $engine->getDatabase();
		$query = $this->query_post;
		$cred = $engine->getCredentials();

		//verify the request
		if($request->getParameter('publish') === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		//publish the content
		if($db->query($engine, $query, array('module_id' => $this->id,
					'content_id' => $content['id'],
					'user_id' => $cred->getUserId()))
				=== FALSE)
			return _('Internal server error');
		return FALSE;
	}

	protected function _publishSuccess($engine, $request, $page, $content)
	{
		$r = new Request($engine, $this->name, FALSE, $content['id'],
				$content['title']);
		$this->helperRedirect($engine, $r, $page,
				$this->text_content_publish_progress);
		return $page;
	}


	//ContentModule::callSubmit
	protected function callSubmit($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$user = new User($engine, $cred->getUserId());
		$title = $this->text_content_submit;
		$error = _('Permission denied');

		//check permissions
		if($this->canSubmit($engine, $error) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//create the page
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//toolbar
		$toolbar = $this->getToolbar($engine);
		$page->append($toolbar);
		//process the request
		$content = FALSE;
		if(($error = $this->_submitProcess($engine, $request, $content))
				=== FALSE)
			return $this->_submitSuccess($engine, $request, $page,
					$content);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		//preview
		$this->helperSubmitPreview($engine, $page, $request, $content);
		//form
		$form = $this->formSubmit($engine, $request);
		$page->append($form);
		return $page;
	}

	protected function _submitProcess($engine, $request, &$content)
	{
		//verify the request
		if($request === FALSE
				|| $request->getParameter('submit') === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		//store the content uploaded
		$title = $request->getParameter('title');
		$content = $request->getParameter('content');
		$public = $request->getParameter('public') ? TRUE : FALSE;
		$content = Content::insert($engine, $this->id, $title, $content,
				$public, TRUE);
		if($content === FALSE)
			return _('Internal server error');
		return FALSE;
	}

	protected function _submitSuccess($engine, $request, $page, $content)
	{
		$r = new Request($engine, $this->name, FALSE, $content->getId(),
				$content->getTitle());
		$this->helperRedirect($engine, $r, $page,
				$this->text_content_submit_progress);
		return $page;
	}


	//ContentModule::callUpdate
	protected function callUpdate($engine, $request)
	{
		//obtain the content
		$error = _('Unable to fetch content');
		if(($content = $this->_get($engine, $request->getId()))
				=== FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//check permissions
		$error = _('Permission denied');
		if($this->canUpdate($engine, $content, $error) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//create the page
		$title = _('Update ').$content['title'];
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//toolbar
		$toolbar = $this->getToolbar($engine, $content);
		$page->append($toolbar);
		//process the request
		if(($error = $this->_updateProcess($engine, $request, $content))
				=== FALSE)
			return $this->_updateSuccess($engine, $request, $page,
					$content);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		//preview
		if($request->getParameter('preview') !== FALSE)
			$this->helperUpdatePreview($engine, $request, $page,
					$content);
		$form = $this->formUpdate($engine, $request, $content);
		$page->append($form);
		return $page;
	}

	protected function _updateProcess($engine, $request, &$content)
	{
		//verify the request
		if($request->getParameter('submit') === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		//update the content
		$title = $request->getParameter('title');
		$text = $request->getParameter('content');
		if(Content::update($engine, $content['id'], $title, $text)
				=== FALSE)
			return _('Internal server error');
		$content['title'] = $title;
		$content['content'] = $text;
		return FALSE;
	}

	protected function _updateSuccess($engine, $request, $page, $content)
	{
		$r = new Request($engine, $this->name, FALSE, $content['id'],
				$content['title']);
		$this->helperRedirect($engine, $r, $page,
				$this->text_content_update_progress);
		return $page;
	}


	//helpers
	//ContentModule::helperAction
	protected function helperAction($engine, $stock, $request, $text)
	{
		$icon = new PageElement('image', array('stock' => $stock));
		$link = new PageElement('link', array('request' => $request,
				'text' => $text));
		return new PageElement('row', array('icon' => $icon,
				'label' => $link));
	}


	//ContentModule::helperActions
	protected function helperActions($engine, $request)
	{
		return FALSE;
	}


	//ContentModule::helperActionsAdmin
	protected function helperActionsAdmin($engine, $request)
	{
		$ret = array();
		$admin = $request->getParameter('admin');

		if($admin === 0)
			return $ret;
		$r = new Request($engine, $this->name, 'admin');
		$ret[] = $this->helperAction($engine, 'admin', $r,
				$this->text_content_admin);
		return $ret;
	}


	//ContentModule::helperActionsSubmit
	protected function helperActionsSubmit($engine, $request)
	{
		$ret = array();

		$r = new Request($engine, $this->name, 'submit');
		$ret[] = $this->helperAction($engine, 'new', $r,
				$this->text_content_submit);
		return $ret;
	}


	//ContentModule::helperActionsUser
	protected function helperActionsUser($engine, $request, $user)
	{
		$ret = array();
		$request = new Request($engine, $this->name, 'list',
				$user->getUserId(), $user->getUsername());
		$ret[] = $this->helperAction($engine, $this->name, $request,
				$this->text_content_list_title_by
				.' '.$user->getUsername());
		return $ret;
	}


	//ContentModule::helperAdminButtons
	protected function helperAdminButtons($engine, $page, $request)
	{
		$r = new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back to this module')));
		$r = new Request($engine, 'admin');
		$page->append('link', array('request' => $r, 'stock' => 'admin',
				'text' => _('Back to the administration')));
	}


	//ContentModule::helperAdminRow
	protected function helperAdminRow($engine, $row, $res)
	{
		$db = $engine->getDatabase();
		$no = new PageElement('image', array('stock' => 'no',
				'size' => 16, 'title' => _('Disabled')));
		$yes = new PageElement('image', array('stock' => 'yes',
				'size' => 16, 'title' => _('Enabled')));

		$row->setProperty('id', 'content_id:'.$res['id']);
		$row->setProperty('icon', '');
		$r = new Request($engine, $this->name, 'update',
				$res['id'], $res['title']);
		$link = new PageElement('link', array('request' => $r,
				'stock' => $this->name,
				'text' => $res['title']));
		$row->setProperty('title', $link);
		$row->setProperty('enabled', $db->isTrue($res['enabled'])
				? $yes : $no);
		$row->setProperty('public', $db->isTrue($res['public'])
				? $yes : $no);
		$r = new Request($engine, 'user', FALSE,
				$res['user_id'], $res['username']);
		$link = new PageElement('link', array('request' => $r,
				'stock' => 'user',
				'text' => $res['username']));
		$row->setProperty('username', $link);
		$date = $db->formatDate($engine, $res['timestamp']);
		$row->setProperty('date', $date);
	}


	//ContentModule::helperAdminToolbar
	protected function helperAdminToolbar($engine, $page, $request)
	{
		$r = new Request($engine, $this->name, 'admin');
		if(($type = $request->getParameter('type')) !== FALSE)
			$r->setParameter('type', $type);
		$toolbar = $page->append('toolbar');
		$toolbar->append('button', array('stock' => 'refresh',
					'text' => _('Refresh'),
					'request' => $r));
		//disable
		$toolbar->append('button', array('stock' => 'disable',
					'text' => _('Disable'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'disable'));
		//enable
		$toolbar->append('button', array('stock' => 'enable',
					'text' => _('Enable'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'enable'));
		//post
		$toolbar->append('button', array('stock' => 'post',
					'text' => _('Post'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'post'));
		//unpost
		$toolbar->append('button', array('stock' => 'unpost',
					'text' => _('Unpost'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'unpost'));
		//delete
		$toolbar->append('button', array('stock' => 'delete',
					'text' => _('Delete'),
					'type' => 'submit', 'name' => 'action',
					'value' => 'delete'));
	}


	//ContentModule::helperApply
	protected function helperApply($engine, $request, $query, $fallback,
			$success, $failure)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(($uid = $cred->getUserId()) == 0)
		{
			//must be logged in
			$page = $this->callDefault($engine);
			$error = _('Must be logged in');
			$page->prepend('dialog', array('type' => 'error',
						'text' => $error));
			return $page;
		}
		//prepare the fallback request
		$fallback = 'call'.ucfirst($fallback);
		$r = new Request($engine, $request->getModule(),
				$request->getAction(), $request->getId(),
				$request->getTitle());
		if(($type = $request->getParameter('type')) !== FALSE)
			$r->setParameter('type', $type);
		//verify the request
		if($request->isIdempotent())
			return $this->$fallback($engine, $r);
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
		$page = $this->$fallback($engine, $r);
		//FIXME place this under the title
		$page->prepend('dialog', array('type' => $type,
					'text' => $message));
		return $page;
	}


	//ContentModule::helperDisplay
	protected function helperDisplay($engine, $page, $content = FALSE)
	{
		if($content === FALSE)
			return FALSE;
		//link
		$request = new Request($engine, $content['module'], FALSE,
				$content['id'], $content['title']);
		//title
		$this->helperDisplayTitle($engine, $page, $request, $content);
		//toolbar
		//FIXME pages should render as vbox by default
		$vbox = $page->append('vbox');
		$this->helperDisplayToolbar($engine, $vbox, $request, $content);
		//meta-data
		$this->helperDisplayMetadata($engine, $vbox, $request,
				$content);
		//text
		$this->helperDisplayText($engine, $vbox, $request, $content);
		//buttons
		$this->helperDisplayButtons($engine, $vbox, $request, $content);
		return $page;
	}


	//ContentModule::helperDisplayButtons
	protected function helperDisplayButtons($engine, $page, $request,
			$content)
	{
		$hbox = $page->append('hbox');
		$r = new Request($engine, $this->name);
		$hbox->append('link', array('stock' => 'back', 'request' => $r,
				'text' => $this->text_content_more_content));
		$hbox->append('link', array('stock' => 'link',
				'request' => $request,
				'text' => $this->text_content_link));
	}


	//ContentModule::helperDisplayMetadata
	protected function helperDisplayMetadata($engine, $page, $request,
			$content)
	{
		$r = new Request($engine, 'user', FALSE,
				$content['user_id'], $content['username']);
		$link = new PageElement('link', array('request' => $r,
				'text' => $content['username']));
		$meta = $page->append('label', array(
				'text' => $this->text_content_by.' '));
		$meta->append($link);
		$meta = $meta->append('label', array(
				'text' => ' '.$this->text_content_on.' '
				.$content['date']));
	}


	//ContentModule::helperDisplayText
	protected function helperDisplayText($engine, $page, $request,
			$content)
	{
		$text = $content['content'];

		$page->append('label', array('text' => $text));
	}


	//ContentModule::helperDisplayTitle
	protected function helperDisplayTitle($engine, $page, $request,
			$content)
	{
		$title = $content['title'];

		$page->setProperty('title', $title);
		$title = $page->append('title', array('stock' => $this->name,
				'text' => $title));
	}


	//ContentModule::helperDisplayToolbar
	protected function helperDisplayToolbar($engine, $page, $request,
			$content)
	{
		$toolbar = $this->getToolbar($engine, $content);
		$page->append($toolbar);
	}


	//ContentModule::helperListButtons
	protected function helperListButtons($engine, $page, $request)
	{
		$user = new User($engine, $request->getId(),
				$request->getTitle());

		if(($uid = $user->getUserId()) == 0)
			$uid = FALSE;
		$r = ($uid !== FALSE)
			? new Request($engine, 'user', 'display',
					$user->getUserId(),
					$user->getUsername())
			: new Request($engine, $this->name);
		$page->append('link', array('request' => $r, 'stock' => 'back',
				'text' => _('Back')));
	}


	//ContentModule::helperListTitle
	protected function helperListTitle($engine, $page, $request)
	{
		$title = $page->getProperty('title');

		$page->append('title', array('stock' => $this->name,
				'text' => $title));
	}


	//ContentModule::helperListToolbar
	protected function helperListToolbar($engine, $page, $request)
	{
		$cred = $engine->getCredentials();
		$user = new User($engine, $request->getId(),
				$request->getTitle());

		if(($uid = $user->getUserId()) == 0)
			$uid = FALSE;
		$r = new Request($engine, $this->name, 'list', $uid,
				$uid ? $user->getUsername() : FALSE);
		$toolbar = $page->append('toolbar');
		$toolbar->append('button', array('stock' => 'refresh',
				'text' => _('Refresh'),
				'request' => $r));
		$r = new Request($engine, $this->name, 'submit');
		if($this->canSubmit($engine))
			$toolbar->append('button', array('stock' => 'new',
					'request' => $r,
					'text' => $this->text_content_submit));
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
	}


	//ContentModule::helperPaging
	protected function helperPaging($engine, $request, $page, $pcnt)
	{
		if($pcnt === FALSE || $this->content_list_count <= 0
				|| ($pcnt <= $this->content_list_count))
			return;
		$sep = '';
		if(($pcur = $request->getParameter('page')) === FALSE)
			$pcur = 1;
		$pcnt = ceil($pcnt / $this->content_list_count);
		for($i = 1; $i <= $pcnt; $i++, $sep = ' | ')
		{
			if(strlen($sep))
				$page->append('label', array('text' => $sep));
			if($i == $pcur)
			{
				$page->append('label', array('text' => $i));
				continue;
			}
			$r = new Request($engine, $this->name, FALSE, FALSE,
					FALSE, array('page' => $i));
			$page->append('link', array('request' => $r,
					'text' => $i));
		}
	}


	//ContentModule::helperPreview
	protected function helperPreview($engine, $preview, $content = FALSE)
	{
		if($content === FALSE)
			return;
		$request = isset($content['id'])
			? new Request($engine, $this->name, FALSE,
				$content['id'], $content['title'])
			: FALSE;
		//title
		$this->helperPreviewTitle($engine, $preview, $request,
				$content);
		//meta-data
		//FIXME pages should render as vbox by default
		$vbox = $preview->append('vbox');
		$this->helperPreviewMetadata($engine, $vbox, $request,
				$content);
		//text
		$this->helperPreviewText($engine, $vbox, $request, $content);
		//buttons
		$this->helperPreviewButtons($engine, $vbox, $request, $content);
	}


	//ContentModule::helperPreviewButtons
	protected function helperPreviewButtons($engine, $preview, $request,
			$content)
	{
		if($request === FALSE)
			return;
		$preview->append('button', array('request' => $request,
				'stock' => $this->content_open_stock,
				'text' => $this->text_content_open));
	}


	//ContentModule::helperPreviewHeader
	protected function helperPreviewHeader($engine, $request, $page)
	{
		$vbox = new PageElement('vbox');

		$page->append($vbox);
		return $vbox;
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
		if(($len = $this->content_preview_length) > 0
				&& strlen($text) > $len)
			//abbreviate text as required
			$text = substr($text, 0, $len).'...';
		$preview->append('label', array('text' => $text));
	}


	//ContentModule::helperPreviewTitle
	protected function helperPreviewTitle($engine, $preview, $request,
			$content)
	{
		if($request === FALSE)
		{
			$preview->append('title', array(
					'text' => $content['title']));
			return;
		}
		$link = new PageElement('link', array('request' => $request,
				'text' => $content['title']));
		$title = $preview->append('title');
		$title->append($link);
	}


	//ContentModule::helperRedirect
	protected function helperRedirect($engine, $request, $page,
			$text = FALSE)
	{
		if($text === FALSE)
			$text = _('Redirection in progress, please wait...');
		$page->setProperty('location', $engine->getUrl($request));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$box->append('label', array('text' => $text));
		$box = $box->append('hbox');
		$text = _('If you are not redirected within 30 seconds, please ');
		$box->append('label', array('text' => $text));
		$box->append('link', array('text' => _('click here'),
				'request' => $request));
		$box->append('label', array('text' => '.'));
		return $page;
	}


	//ContentModule::helperSubmitButtons
	protected function helperSubmitButtons($engine, $request, $page)
	{
		$r = new Request($engine, $this->name);
		$page->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		if($this->canPreview($engine, $request))
			$page->append('button', array('type' => 'submit',
					'stock' => 'preview',
					'name' => 'action',
					'value' => 'preview',
					'text' => _('Preview')));
		$page->append('button', array('type' => 'submit',
				'stock' => 'submit', 'name' => 'action',
				'value' => 'submit', 'text' => _('Submit')));
	}


	//ContentModule::helperSubmitContent
	protected function helperSubmitContent($engine, $request, $page)
	{
		$page->append('textview', array('name' => 'content',
				'text' => _('Content: '),
				'value' => $request->getParameter('content')));
	}


	//ContentModule::helperSubmitPreview
	protected function helperSubmitPreview($engine, $page, $request,
			$content)
	{
		$cred = $engine->getCredentials();
		$user = new User($engine, $cred->getUserId());

		if($request === FALSE
				|| $request->getParameter('preview') === FALSE)
			return;
		$content = $request->getParameter('content');
		$content = array('title' => _('Preview: ')
					.$request->getParameter('title'),
				'user_id' => $user->getUserId(),
				'username' => $user->getUsername(),
				'date' => $this->timestampToDate(),
				'content' => $content);
		$this->helperPreview($engine, $page, $content);
	}


	//ContentModule::helperSubmitTitle
	protected function helperSubmitTitle($engine, $request, $page)
	{
		$page->append('entry', array('name' => 'title',
				'text' => _('Title: '),
				'value' => $request->getParameter('title')));
	}


	//ContentModule::helperUpdateContent
	protected function helperUpdateContent($engine, $request, $page,
			$content)
	{
		$page->append('label', array('text' => _('Content: ')));
		if(($value = $request->getParameter('content')) === FALSE)
			$value = $content['content'];
		$page->append('textview', array('name' => 'content',
				'value' => $value));
	}


	//ContentModule::helperUpdatePreview
	protected function helperUpdatePreview($engine, $request, $page,
			$content)
	{
		$vbox = $page->append('vbox');
		$preview = array('module' => $this->name,
			'user_id' => $content['user_id'],
			'username' => $content['username'],
			'date' => $content['date'],
			'title' => _('Preview: ')
				.$request->getParameter('title'),
			'content' => $request->getParameter('content'));
		$this->helperPreview($engine, $vbox, $preview);
	}


	//ContentModule::helperUpdateTitle
	protected function helperUpdateTitle($engine, $request, $page, $content)
	{
		if(($value = $request->getParameter('title')) === FALSE)
			$value = $content['title'];
		$page->append('entry', array('name' => 'title',
				'text' => _('Title: '), 'value' => $value));
	}


	//ContentModule::helperUpdateButtons
	protected function helperUpdateButtons($engine, $request, $page,
			$content)
	{
		$hbox = $page->append('hbox');
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
				'stock' => 'update', 'name' => 'action',
				'value' => 'submit', 'text' => _('Update')));
	}
}

?>
