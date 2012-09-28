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



require_once('./system/auth.php');
require_once('./system/module.php');
require_once('./system/user.php');


//UserModule
class UserModule extends Module
{
	//public
	//methods
	//essential
	//UserModule::UserModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('Users') : $title;
		parent::__construct($id, $name, $title);
	}


	//useful
	//UserModule::call
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
			case 'login':
			case 'logout':
			case 'profile':
			case 'register':
			case 'reset':
			case 'submit':
			case 'update':
			case 'validate':
			case 'widget':
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return FALSE;
	}


	//protected
	//properties
	protected $module_name = 'User';


	//methods
	//accessors
	//UserModule::canRegister
	protected function canRegister()
	{
		global $config;

		return $config->getVariable('module::user', 'register') == 1;
	}


	//UserModule::canReset
	protected function canReset()
	{
		global $config;

		return $config->getVariable('module::user', 'reset') == 1;
	}


	//UserModule::canSubmit
	protected function canSubmit($engine, &$error)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
		{
			$error = _('Permission denied');
			return FALSE;
		}
		return TRUE;
	}


	//forms
	//UserModule::formLogin
	protected function formLogin($engine, $username, $cancel = TRUE)
	{
		$r = new Request($this->name, 'login');
		$form = new PageElement('form', array('request' => $r));
		$entry = $form->append('entry', array(
					'name' => 'username',
					'text' => _('Username: '),
					'value' => $username));
		$entry = $form->append('entry', array(
					'hidden' => TRUE,
					'name' => 'password',
					'text' => _('Password: ')));
		$r = new Request($this->name);
		if($cancel)
			$form->append('button', array('text' => _('Cancel'),
						'stock' => 'cancel',
						'request' => $r));
		$button = $form->append('button', array('type' => 'submit',
					'stock' => 'login',
					'text' => _('Login')));
		return $form;
	}


	//UserModule::formRegister
	protected function formRegister($engine, $username, $email)
	{
		$r = new Request($this->name, 'register');
		$form = new PageElement('form', array('request' => $r));
		$form->append('entry', array('text' => _('Username: '),
				'name' => 'username', 'value' => $username));
		$form->append('entry', array('text' => _('e-mail address: '),
				'name' => 'email', 'value' => $email));
		$form->append('button', array('stock' => 'cancel',
				'text' => _('Cancel'),
				'request' => new Request($this->name)));
		$form->append('button', array('stock' => 'register',
				'type' => 'submit', 'text' => _('Register')));
		return $form;
	}


	//UserModule::formReset
	protected function formReset($engine, $username, $email)
	{
		$r = new Request($this->name, 'reset');
		$form = new PageElement('form', array('request' => $r));
		$form->append('entry', array('text' => _('Username: '),
				'name' => 'username', 'value' => $username));
		$form->append('entry', array('text' => _('e-mail address: '),
				'name' => 'email', 'value' => $email));
		$form->append('button', array('stock' => 'cancel',
				'text' => _('Cancel'),
				'request' => new Request($this->name)));
		$form->append('button', array('stock' => 'reset',
				'type' => 'submit', 'text' => _('Reset')));
		return $form;
	}


	//UserModule::formSubmit
	protected function formSubmit($engine, $request)
	{
		$r = new Request($this->name, 'submit');
		$form = new PageElement('form', array('request' => $r));
		$vbox = $form->append('vbox');
		$vbox->append('entry', array('name' => 'username',
				'text' => _('Username: '),
				'value' => $request->getParameter('username')));
		$vbox->append('entry', array('name' => 'fullname',
				'text' => _('Full name: '),
				'value' => $request->getParameter('fullname')));
		$vbox->append('entry', array('name' => 'password',
				'hidden' => TRUE,
				'text' => _('Password: '), 'value' => ''));
		$vbox->append('entry', array('name' => 'email',
				'text' => _('e-mail: '),
				'value' => $request->getParameter('email')));
		//enabled
		$vbox->append('checkbox', array('name' => 'enabled',
				'value' => $request->getParameter('enabled')
					? TRUE : FALSE,
				'text' => _('Enabled')));
		//administrator
		$vbox->append('checkbox', array('name' => 'admin',
				'value' => $request->getParameter('admin')
					? TRUE : FALSE,
				'text' => _('Administrator')));
		//buttons
		$r = new Request($this->name, 'admin');
		$form->append('button', array('request' => $r,
				'stock' => 'cancel', 'text' => _('Cancel')));
		$form->append('button', array('type' => 'submit',
				'stock' => 'new', 'name' => 'action',
				'value' => 'submit', 'text' => _('Create')));
		return $form;
	}


	//UserModule::formUpdate
	protected function formUpdate($engine, $request, $user, $id, $error)
	{
		$cred = $engine->getCredentials();

		//output the page
		$title = $id ? _('Profile update for ').$user->getUsername()
			: _('Profile update');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'user',
				'text' => $title));
		if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		$r = new Request($this->name, 'update', $request->getId(),
		       	$request->getId() ? $request->getTitle() : FALSE);
		$form = $page->append('form', array('request' => $r));
		//fields
		//username (cannot be changed)
		$form->append('label', array('text' => _('Username: ')));
		$form->append('label', array('text' => $user->getUsername()));
		//full name
		if(($fullname = $request->getParameter('fullname')) === FALSE)
			$fullname = $user->getFullname();
		$form->append('entry', array('text' => _('Full name: '),
				'name' => 'fullname', 'value' => $fullname));
		//e-mail address
		if(($email = $request->getParameter('email')) === FALSE)
			$email = $user->getEmail();
		$form->append('entry', array('text' => _('e-mail: '),
				'name' => 'email', 'value' => $email));
		//password
		$form->append('label', array('text' => _('Optionally: ')));
		if($id === FALSE && !$cred->isAdmin())
			$form->append('entry', array(
				'text' => _('Current password: '),
				'name' => 'password', 'hidden' => TRUE));
		$form->append('entry', array('text' => _('New password: '),
				'name' => 'password1', 'hidden' => TRUE));
		$form->append('entry', array(
				'text' => _('Repeat new password: '),
				'name' => 'password2', 'hidden' => TRUE));
		//buttons
		if($cred->isAdmin() && $request->getId() !== FALSE)
			$r = new Request($this->name, 'admin');
		else
			$r = new Request($this->name, 'profile',
					$request->getId(), $request->getId()
					? $user->getUsername() : FALSE);
		$form->append('button', array('stock' => 'cancel',
				'request' => $r, 'text' => _('Cancel')));
		$form->append('button', array('stock' => 'update',
				'type' => 'submit', 'text' => _('Update')));
		return $page;
	}


	//useful
	//UserModule::actions
	protected function actions($engine, $request)
	{
		$cred = $engine->getCredentials();

		if($request->getParameter('user') !== FALSE)
			return FALSE;
		$ret = array();
		if($request->getParameter('admin'))
			return $this->_actions_admin($engine, $cred,
					$this->name, $ret);
		if($cred->getUserId() == 0)
		{
			//not logged in yet
			$r = new Request($this->name, 'login');
			$icon = new PageElement('image', array(
					'stock' => 'login'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('Login')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'important' => TRUE,
					'label' => $link));
			if($this->canReset())
			{
				$r = new Request($this->name, 'reset');
				$icon = new PageElement('image', array(
						'stock' => 'reset'));
				$link = new PageElement('link', array(
						'request' => $r,
						'text' => _('Password reset')));
				$ret[] = new PageElement('row', array(
						'icon' => $icon,
						'label' => $link));
			}
			if($this->canRegister())
			{
				$r = new Request($this->name, 'register');
				$icon = new PageElement('image', array(
						'stock' => 'register'));
				$link = new PageElement('link', array(
						'request' => $r,
						'text' => _('Register')));
				$ret[] = new PageElement('row', array(
						'icon' => $icon,
						'label' => $link));
			}
		}
		else
		{
			//already logged in
			if($request->getParameter('admin') != 0)
				$this->_actions_admin($engine, $cred,
						$this->name, $ret);
			//user's content
			$r = new Request($this->name, 'display');
			$icon = new PageElement('image', array(
					'stock' => 'user'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('My content')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'label' => $link));
			//user's profile
			$r = new Request($this->name, 'profile');
			$icon = new PageElement('image', array(
					'stock' => 'user'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('My profile')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'label' => $link));
			//logout
			$r = new Request($this->name, 'logout');
			$icon = new PageElement('image', array(
					'stock' => 'logout'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('Logout')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'important' => TRUE,
					'label' => $link));
		}
		return $ret;
	}

	private function _actions_admin($engine, $cred, $module, &$ret)
	{
		if(!$cred->isAdmin())
			return $ret;
		//user creation
		$r = new Request($module, 'submit');
		$icon = new PageElement('image', array('stock' => 'new'));
		$link = new PageElement('link', array('request' => $r,
				'text' => _('New user')));
		$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		//administration
		$r = new Request($module, ($module == 'admin')
			? FALSE : 'admin');
		$icon = new PageElement('image', array('stock' => 'admin'));
		$link = new PageElement('link', array('request' => $r,
				'text' => _('Users administration')));
		$ret[] = new PageElement('row', array('icon' => $icon,
				'label' => $link));
		return $ret;
	}


	//calls
	//UserModule::callAdmin
	protected function callAdmin($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$cred = $engine->getCredentials();
		$actions = array('delete', 'disable', 'enable');

		if(!$cred->isAdmin())
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('Permission denied')));
		//perform actions if necessary
		if($request !== FALSE)
			foreach($actions as $a)
				if($request->getParameter($a) !== FALSE)
				{
					$a = 'call'.ucfirst($a);
					return $this->$a($engine, $request);
				}
		//list users
		$title = _('User administration');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$query = $this->query_admin;
		//FIXME implement sorting
		$query .= ' ORDER BY username ASC';
		if(($res = $db->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('Could not list users')));
		$columns = array('username' => _('Username'),
				'group' => _('Group'),
				'enabled' => _('Enabled'),
				'admin' => _('Administrator'),
				'email' => _('e-mail'));
		$r = new Request($this->name, 'admin');
		$view = $page->append('treeview', array('request' => $r,
				'view' => 'details', 'columns' => $columns));
		//toolbar
		$toolbar = $view->append('toolbar');
		$toolbar->append('button', array('stock' => 'new',
				'text' => _('New user'),
				'request' => new Request($this->name,
					'submit')));
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
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $view->append('row');
			$row->setProperty('id', 'user_id:'.$res[$i]['id']);
			$row->setProperty('username', $res[$i]['username']);
			$r = new Request($this->name, 'update', $res[$i]['id'],
				$res[$i]['username']);
			$link = new PageElement('link', array('stock' => 'user',
					'request' => $r,
					'text' => $res[$i]['username']));
			if($res[$i]['id'] != 0)
				$row->setProperty('username', $link);
			$row->setProperty('group', $res[$i]['groupname']);
			$row->setProperty('enabled', $db->isTrue(
					$res[$i]['enabled']) ? $yes : $no);
			$row->setProperty('admin', $db->isTrue(
					$res[$i]['admin']) ? $yes : $no);
			$link = new PageElement('link', array(
					'url' => 'mailto:'.$res[$i]['email'],
					'text' => $res[$i]['email']));
			$row->setProperty('email', $link);
		}
		$vbox = $page->append('vbox');
		$r = new Request($this->name);
		$vbox->append('link', array('request' => $r, 'stock' => 'back',
			'text' => _('Back to my account')));
		$r = new Request('admin');
		$vbox->append('link', array('request' => $r, 'stock' => 'admin',
			'text' => _('Back to the administration')));
		return $page;
	}


	//UserModule::callDefault
	protected function callDefault($engine, $request = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $this->query_content;
		$cred = $engine->getCredentials();

		if($request !== FALSE && ($id = $request->getId()) !== FALSE)
			return $this->callDisplay($engine, $request);
		//FIXME add content?
		$title = ($cred->getUserId() != 0) ? _('My account')
			: _('Site menu');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//obtain the list of modules
		if(($res = $db->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$vbox = $page->append('vbox');
		$vbox->append('title'); //XXX to reduce the next level of titles
		$vbox = $vbox->append('vbox');
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$r = new Request($res[$i]['name'], 'actions', FALSE,
					FALSE, array('admin' => 0));
			$rows = $engine->process($r);
			if(!is_array($rows) || count($rows) == 0)
				continue;
			$r = new Request($res[$i]['name']);
			$text = ucfirst($res[$i]['name']);
			$link = new PageElement('link', array('request' => $r,
					'text' => $text));
			$title = $vbox->append('title', array(
					'stock' => $res[$i]['name']));
			$title->append($link);
			$view = $vbox->append('iconview');
			foreach($rows as $r)
				$view->append($r);
		}
		$r = new Request();
		$page->append('link', array('stock' => 'back', 'request' => $r,
				'text' => _('Back to the site')));
		return $page;
	}


	//UserModule::callDelete
	protected function callDelete($engine, $request)
	{
		$query = $this->query_delete;

		return $this->helperApply($engine, $request, $query, 'admin',
			_('User(s) could be deleted successfully'),
			_('Some user(s) could not be deleted'));
	}


	//UserModule::callDisable
	protected function callDisable($engine, $request)
	{
		$query = $this->query_disable;

		return $this->helperApply($engine, $request, $query, 'admin',
			_('User(s) could be disabled successfully'),
			_('Some user(s) could not be disabled'));
	}


	//UserModule::callDisplay
	protected function callDisplay($engine, $request)
	{
		$database = $engine->getDatabase();
		$query = $this->query_content;
		$cred = $engine->getCredentials();
		$link = FALSE;

		//obtain the list of modules
		if(($res = $database->query($engine, $query)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => 'Could not list modules'));
		$page = new Page;
		if(($uid = $request->getId()) !== FALSE)
		{
			$user = new User($engine, $uid, $request->getTitle());
			$uid = $user->getUserId();
			$title = _('Content from ').$user->getUsername();
		}
		else if(($uid = $cred->getUserId()) != 0)
		{
			$user = new User($engine, $uid);
			$uid = $user->getUserId('id');
			$title = _('My content');
			$r = new Request($this->name);
			$link = new PageElement('link', array('stock' => 'back',
					'request' => $r,
					'text' => _('Back to my account')));
		}
		if($uid == 0)
			return $this->callLogin($engine, new Request);
		//title
		$page->setProperty('title', $title);
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$vbox = $page->append('vbox');
		$vbox->append('title'); //XXX to reduce the next level of titles
		$vbox = $vbox->append('vbox');
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$r = new Request($res[$i]['name'], 'actions', FALSE,
				FALSE, array('user' => $user));
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
		//buttons
		if($link !== FALSE)
			$page->append($link);
		return $page;
	}


	//UserModule::callEnable
	protected function callEnable($engine, $request)
	{
		$query = $this->query_enable;

		return $this->helperApply($engine, $request, $query, 'admin',
			_('User(s) could be enabled successfully'),
			_('Some user(s) could not be enabled'));
	}


	//UserModule::callLogin
	protected function callLogin($engine, $request)
	{
		$cred = $engine->getCredentials();
		$title = _('User login');
		$already = _('You are already logged in');
		$forgot = _('I forgot my password...');
		$register = _('Register an account...');

		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'login',
				'text' => $title));
		//process login
		$error = $this->_loginProcess($engine, $request);
		//login successful
		if($error === FALSE)
			return $this->_loginSuccess($engine, $request, $page);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
						'text' => $error));
		else if($cred->getUserId() != 0)
			$page->append('dialog', array('type' => 'info',
						'text' => $already));
		$form = $this->formLogin($engine,
				$request->getParameter('username'));
		$page->append($form);
		if($this->canReset())
		{
			$r = new Request($this->name, 'reset');
			$page->append('link', array('request' => $r,
					'stock' => 'reset',
					'text' => $forgot));
		}
		if($this->canRegister())
		{
			$r = new Request($this->name, 'register');
			$page->append('link', array('request' => $r,
					'stock' => 'register',
					'text' => $register));
		}
		return $page;
	}

	protected function _loginProcess($engine, $request)
	{
		$db = $engine->getDatabase();

		if(($username = $request->getParameter('username')) === FALSE
				|| strlen($username) == 0
				|| ($password = $request->getParameter(
						'password')) === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		if(($user = User::lookup($engine, $username)) === FALSE
				|| ($cred = $user->authenticate($engine,
					$password)) === FALSE)
			return _('Invalid username or password');
		if($engine->setCredentials($cred) !== TRUE)
			return _('An error occurred while authenticating');
		return FALSE;
	}

	protected function _loginSuccess($engine, $request, $page)
	{
		$r = new Request();
		$page->setProperty('location', $engine->getUrl($r));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$text = _('Logging in progress, please wait...');
		$box->append('label', array('text' => $text));
		$box = $box->append('hbox');
		$text = _('If you are not redirected within 30 seconds, please ');
		$box->append('label', array('text' => $text));
		$box->append('link', array('text' => _('click here'),
			'request' => $r));
		$box->append('label', array('text' => '.'));
		return $page;
	}


	//UserModule::callLogout
	protected function callLogout($engine, $request)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		$page->append('title', array('stock' => 'logout',
				'text' => _('User logout')));
		if($cred->getUserId() == 0)
		{
			$text = _('You were logged out successfully');
			$page->append('dialog', array('type' => 'info',
						'text' => $text));
			$r = new Request();
			$page->append('link', array('stock' => 'back',
					'request' => $r,
					'text' => _('Back to the site')));
			return $page;
		}
		$r = new Request($this->name, 'logout');
		if($request->isIdempotent())
		{
			//FIXME make it a question dialog
			$form = $page->append('form', array(
						'request' => $r));
			$vbox = $form->append('vbox');
			$vbox->append('label', array(
				'text' => _('Do you really want to logout?')));
			$r = new Request($this->name);
			$form->append('button', array('text' => _('Cancel'),
						'stock' => 'cancel',
						'request' => $r));
			$form->append('button', array('text' => _('Logout'),
						'stock' => 'logout',
						'type' => 'submit'));
			return $page;
		}
		//process logout
		$page->setProperty('location', $engine->getUrl($r));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$text = _('Logging out, please wait...');
		$box->append('label', array('text' => $text));
		$box = $box->append('hbox');
		$text = 'If you are not redirected within 30 seconds,'
			.' please ';
		$box->append('label', array('text' => $text));
		$box->append('link', array('text' => 'click here',
					'request' => $r));
		$box->append('label', array('text' => '.'));
		$engine->setCredentials(new AuthCredentials);
		return $page;
	}


	//UserModule::callProfile
	protected function callProfile($engine, $request)
	{
		$cred = $engine->getCredentials();
		$id = $request->getId();

		//determine whose profile to view
		if($id === FALSE)
			$id = $cred->getUserId();
		$user = new User($engine, $id, $request->getTitle());
		if(($id = $user->getUserId()) == 0)
		{
			//the anonymous user has no profile
			$error = _('There is no profile for this user');
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		}
		if($id === $cred->getUserId())
			//viewing own profile
			$id = FALSE;
		//output the page
		$title = $id ? _('Profile for ').$user->getUsername()
			: _('My profile');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'user',
				'text' => $title));
		$vbox = $page->append('vbox');
		$hbox = $vbox->append('hbox');
		$col1 = $hbox->append('vbox');
		$col2 = $hbox->append('vbox');
		$col1->append('label', array('class' => 'bold',
				'text' => _('Fullname: ')));
		$col2->append('label', array('text' => $user->getFullname()));
		$col1->append('label', array('class' => 'bold',
				'text' => _('e-mail: ')));
		$col2->append('label', array('text' => $user->getEmail()));
		//link to profile update
		$r = new Request($this->name, 'update', $request->getId(),
			$request->getId() ? $user->getUsername() : FALSE);
		$button = FALSE;
		if($request->getId() !== FALSE && $cred->isAdmin())
			$button = new PageElement('button', array(
				'stock' => 'admin', 'request' => $r,
				'text' => _('Update')));
		else if($id === FALSE)
			$button = new PageElement('button', array(
				'stock' => 'user', 'request' => $r,
				'text' => _('Update')));
		if($button !== FALSE)
			$vbox->append($button);
		if($id === FALSE)
		{
			$r = new Request($this->name);
			$vbox->append('link', array('stock' => 'back',
					'request' => $r,
					'text' => _('Back to my account')));
		}
		return $page;
	}


	//UserModule::callRegister
	protected function callRegister($engine, $request)
	{
		$cred = $engine->getCredentials();
		$error = TRUE;

		if($cred->getUserId() != 0)
			//already registered and logged in
			return $this->callDisplay($engine, new Request);
		//process registration
		if(!$this->canRegister())
			$error = _('Registering is not allowed');
		else if(!$request->isIdempotent())
			$error = $this->_register_process($engine, $request);
		if($error === FALSE)
			//registration was successful
			return $this->_register_success($engine, $request);
		return $this->_register_form($engine, $request, $error);
	}

	private function _register_form($engine, $request, $error)
	{
		$title = _('User registration');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		if(is_string($error))
			$page->append('dialog', array('type' => 'error',
				'text' => $error));
		$username = $request->getParameter('username');
		$email = $request->getParameter('email');
		$form = $this->formRegister($engine, $username, $email);
		$page->append($form);
		return $page;
	}

	private function _register_process($engine, $request)
	{
		$ret = '';

		if(($username = $request->getParameter('username')) === FALSE)
			$ret .= _("A username is required\n");
		if(($email = $request->getParameter('email')) === FALSE)
			$ret .= _("An e-mail address is required\n");
		if(strlen($ret) > 0)
			return $ret;
		//register the user
		$error = '';
		if(($user = User::register($engine, $this->name, $username,
				FALSE, $email, FALSE, $error)) === FALSE)
			$ret .= $error;
		return strlen($ret) ? $ret : FALSE;
	}

	private function _register_success($engine, $request)
	{
		$title = _('User registration');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$page->append('dialog', array('type' => 'info',
				'text' => _("You should receive an e-mail shortly with your password, along with a confirmation key.\n
Thank you for registering!")));
		$page->append('link', array('stock' => 'back',
			'text' => _('Back to the site'),
			'request' => new Request()));
		return $page;
	}


	//UserModule::callReset
	protected function callReset($engine, $request)
	{
		$cred = $engine->getCredentials();
		$error = TRUE;

		if($cred->getUserId() != 0)
			//already registered and logged in
			return $this->callDisplay($engine, new Request);
		if(($uid = $request->getId()) !== FALSE
				&& ($token = $request->getParameter('token'))
				!== FALSE)
			return $this->_reset_token($engine, $request, $uid,
					$token);
		//process reset
		if(!$this->canReset())
			$error = _('Password resets are not allowed');
		else if(!$request->isIdempotent())
			$error = $this->_reset_process($engine, $request);
		if($error === FALSE)
			//reset was successful
			return $this->_reset_success($engine, $request);
		return $this->_reset_form($engine, $request, $error);
	}

	private function _reset_form($engine, $request, $error)
	{
		$title = _('Password reset');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		if(is_string($error))
			$page->append('dialog', array('type' => 'error',
				'text' => $error));
		$username = $request->getParameter('username');
		$email = $request->getParameter('email');
		$form = $this->formReset($engine, $username, $email);
		$page->append($form);
		return $page;
	}

	private function _reset_process($engine, $request)
	{
		$ret = '';

		if(($username = $request->getParameter('username')) === FALSE)
			$ret .= _("Your username is required\n");
		if(($email = $request->getParameter('email')) === FALSE)
			$ret .= _("Your e-mail address is required\n");
		if(strlen($ret) > 0)
			return $ret;
		//send a reset token to the user
		$error = '';
		if(($user = User::reset($engine, $this->name, $username, $email,
				$error)) === FALSE)
			$ret .= $error;
		return strlen($ret) ? $ret : FALSE;
	}

	private function _reset_success($engine, $request)
	{
		$title = _('Password reset');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$page->append('dialog', array('type' => 'info',
				'text' => _("You should receive an e-mail shortly, with a link allowing you to reset your password.\n")));
		$page->append('link', array('stock' => 'back',
			'text' => _('Back to the site'),
			'request' => new Request()));
		return $page;
	}

	private function _reset_token($engine, $request, $uid, $token)
	{
		$error = TRUE;

		//process reset
		if(!$this->canReset())
			$error = _('Password resets are not allowed');
		else if(!$request->isIdempotent())
			$error = $this->_reset_token_process($engine, $request,
					$uid, $token);
		if($error === FALSE)
			//reset was successful
			return $this->_reset_token_success($engine, $request);
		return $this->_reset_token_form($engine, $request, $uid, $token,
				$error);
	}

	private function _reset_token_form($engine, $request, $uid, $token,
			$error)
	{
		$title = _('Password reset');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		if(is_string($error))
			$page->append('dialog', array('type' => 'error',
				'text' => $error));
		$r = new Request($this->name, 'reset', FALSE, FALSE,
			array('id' => $uid, 'token' => $token));
		$form = $page->append('form', array('request' => $r));
		$token = $request->getParameter('token');
		$form->append('entry', array('text' => _('Password: '),
			'name' => 'password'));
		$form->append('entry', array('text' => _('Repeat password: '),
			'name' => 'password2'));
		$form->append('button', array('stock' => 'cancel',
			'text' => _('Cancel'),
			'request' => new Request($this->name)));
		$form->append('button', array('stock' => 'reset',
			'type' => 'submit', 'text' => _('Reset')));
		return $page;
	}

	private function _reset_token_process($engine, $request, $uid, $token)
	{
		$ret = '';

		if(($password = $request->getParameter('password')) === FALSE)
			$ret .= _('A new password is required');
		else if(($password2 = $request->getParameter('password2'))
					=== FALSE
					|| $password !== $password2)
			$ret .= _('The passwords did not match');
		if(strlen($ret) > 0)
			return $ret;
		//reset the password
		$error = '';
		if(User::resetPassword($engine, $uid, $password, $token, $error)
				=== FALSE)
			$ret .= $error;
		return strlen($ret) ? $ret : FALSE;
	}

	private function _reset_token_success($engine, $request)
	{
		$title = _('Password reset');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$page->append('dialog', array('type' => 'info',
				'text' => _("Your password was reset successfully.\n")));
		$page->append('link', array('stock' => 'back',
			'text' => _('Back to the site'),
			'request' => new Request()));
		$page->append('link', array('stock' => 'login',
			'text' => _('Proceed to login page'),
			'request' => new Request($this->name)));
		return $page;
	}


	//UserModule::callSubmit
	protected function callSubmit($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();
		$title = _('New user');
		$error = _('Permission denied');

		//check permissions
		if($this->canSubmit($engine, $error) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//create the page
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//process the request
		$user = FALSE;
		if(($error = $this->_submitProcess($engine, $request, $user))
				=== FALSE)
			return $this->_submitSuccess($engine, $request, $page,
					$user);
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		//form
		$form = $this->formSubmit($engine, $request);
		$page->append($form);
		return $page;
	}

	protected function _submitProcess($engine, $request, &$user)
	{
		//verify the request
		if($request === FALSE
				|| $request->getParameter('submit') === FALSE)
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		if(($username = $request->getParameter('username')) === FALSE)
			return _('Invalid arguments');
		$enabled = $request->getParameter('enabled') ? TRUE : FALSE;
		$admin = $request->getParameter('admin') ? TRUE : FALSE;
		//create the user
		$error = FALSE;
		$user = User::insert($engine, $username,
				$request->getParameter('fullname'),
				$request->getParameter('password'),
				$request->getParameter('email'),
				$enabled, $admin, $error);
		if($user === FALSE)
			return $error;
		return FALSE;
	}

	protected function _submitSuccess($engine, $request, $page, $user)
	{
		$r = new Request($this->name, FALSE, $user->getUserId(),
			$user->getUsername());
		$this->helperRedirect($engine, $r, $page);
		return $page;
	}


	//UserModule::callUpdate
	protected function callUpdate($engine, $request)
	{
		$cred = $engine->getCredentials();
		$id = $request->getId();
		$error = TRUE;

		//determine whose profile to update
		if($id === FALSE)
			$user = new User($engine, $cred->getUserId());
		else
			$user = new User($engine, $id, $request->getTitle());
		if(($id = $user->getUserId()) == 0)
		{
			//the anonymous user has no profile
			$error = _('There is no profile for this user');
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		}
		if($id === $cred->getUserId())
			//viewing own profile
			$id = FALSE;
		if($id !== FALSE && !$cred->isAdmin())
		{
			$error = _('Permission denied');
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		}
		//process update
		if(!$request->isIdempotent())
			$error = $this->_updateProcess($engine, $request,
					$user);
		if($error === FALSE)
			//update was successful
			return $this->_updateSuccess($engine, $request);
		return $this->formUpdate($engine, $request, $user, $id,
				$error);
	}

	private function _updateProcess($engine, $request, $user)
	{
		$ret = '';
		$db = $engine->getDatabase();
		$cred = $engine->getCredentials();

		if(($fullname = $request->getParameter('fullname')) === FALSE)
			$ret .= _("The full name is required\n");
		if(($email = $request->getParameter('email')) === FALSE)
			$ret .= _("The e-mail address is required\n");
		if(strlen($ret) > 0)
			return $ret;
		//update the profile
		$error = '';
		if($db->query($engine, $this->query_update, array(
				'user_id' => $user->getUserId(),
				'fullname' => $fullname,
				'email' => $email)) === FALSE)
			return _('Could not update the profile');
		//update the password if requested
		if(($password1 = $request->getParameter('password1')) === FALSE
				|| strlen($password1) == 0
				|| ($password2 = $request->getParameter(
					'password2')) === FALSE
				|| strlen($password2) == 0)
			return FALSE;
		//check the current password (if not an admin)
		if(!$cred->isAdmin())
		{
			$error = _('The current password must be specified');
			if(($password = $request->getParameter('password'))
					=== FALSE
					|| strlen($password) == 0)
				return $error;
			if($user->authenticate($engine, $password) === FALSE)
				return $error;
		}
		//verify that the new password matches
		if($password1 != $password2)
			return _('The new password does not match');
		if(!$user->setPassword($engine, $password1))
			return _('Could not set the new password');
		return FALSE;
	}

	private function _updateSuccess($engine, $request)
	{
		//FIXME also implement administrative updates
		$title = _('Profile update');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$info = _('Your profile was updated successfully');
		$dialog = $page->append('dialog', array('type' => 'info',
				'text' => $info));
		$r = new Request($this->name, 'profile', $request->getId(),
			$request->getId() ? $request->getTitle() : FALSE);
		$dialog->append('button', array('stock' => 'user',
				'request' => $r, 'text' => _('My profile')));
		return $page;
	}


	//UserModule::callValidate
	protected function callValidate($engine, $request)
	{
		$cred = $engine->getCredentials();
		$error = TRUE;
		$uid = $request->getId();
		$token = $request->getParameter('token');

		if($cred->getUserId() != 0)
			//already registered and logged in
			return $this->callDisplay($engine, new Request);
		$page = new Page(array('title' => _('Account confirmation')));
		$page->append('title', array('stock' => $this->name,
				'text' => _('Account confirmation')));
		if(!$this->canRegister())
		{
			$page->append('dialog', array('type' => 'error',
				'text' => _('Registering is not allowed')));
			return $page;
		}
		$box = $page->append('vbox');
		if(($user = User::validate($engine, $uid, $token, $error))
				=== FALSE)
			$box->append('dialog', array('type' => 'error',
					'text' => $error));
		else
		{
			$box->append('dialog', array('type' => 'info',
					'title' => _('Congratulations!'),
					'text' => _("Your account is now enabled.")));
			$r = new Request($this->name);
			$box->append('link', array('stock' => 'login',
					'request' => $r,
					'text' => _('Login')));
		}
		$r = new Request();
		$box->append('link', array('stock' => 'back', 'request' => $r,
			'text' => _('Back to the site')));
		return $page;
	}


	//UserModule::callWidget
	protected function callWidget($engine, $request)
	{
		$cred = $engine->getCredentials();

		$request = $engine->getRequest();
		if($cred->getUserId() == 0)
			return $this->formLogin($engine,
					$request->getParameter('username'),
					FALSE);
		$box = new PageElement('vbox');
		$r = new Request($this->name);
		$box->append('button', array('stock' => 'home',
				'request' => $r,
				'text' => _('My account')));
		$r = new Request($this->name, 'display');
		$box->append('button', array('stock' => 'user',
				'request' => $r,
				'text' => _('My content')));
		$r = new Request($this->name, 'update');
		$box->append('button', array('stock' => 'user',
				'request' => $r,
				'text' => _('My profile')));
		$r = new Request($this->name, 'logout');
		$box->append('button', array('stock' => 'logout',
				'request' => $r,
				'text' => _('Logout')));
		return $box;
	}


	//helpers
	//UserModule::helperApply
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
			if(count($x) != 2 || $x[0] != 'user_id'
					|| !is_numeric($x[1]))
				continue;
			$res = $db->query($engine, $query, array(
					'user_id' => $x[1]));
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


	//UserModule::helperRedirect
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


	//private
	//properties
	//queries
	private $query_admin = 'SELECT user_id AS id, username, admin,
		daportal_user.enabled AS enabled, email,
		daportal_group.group_id AS group_id, groupname
		FROM daportal_user
		LEFT JOIN daportal_group
		ON daportal_user.group_id=daportal_group.group_id';
	private $query_content = "SELECT name FROM daportal_module
		WHERE enabled='1' ORDER BY name ASC";
	private $query_delete = "DELETE FROM daportal_user
		WHERE user_id=:user_id";
	private $query_disable = "UPDATE daportal_user
		SET enabled='0'
		WHERE user_id=:user_id";
	private $query_enable = "UPDATE daportal_user
		SET enabled='1'
		WHERE user_id=:user_id";
	private $query_update = 'UPDATE daportal_user
		SET fullname=:fullname, email=:email
		WHERE user_id=:user_id';
}

?>
