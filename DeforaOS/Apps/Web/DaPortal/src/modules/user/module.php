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
	//UserModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'actions':
			case 'admin':
			case 'display':
			case 'login':
			case 'logout':
			case 'menu':
			case 'register':
			case 'validate':
			case 'widget':
				return $this->$action($engine, $request);
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
	//methods
	//UserModule::actions
	protected function actions($engine, $request)
	{
		$cred = $engine->getCredentials();

		$ret = array();
		if($cred->getUserId() == 0)
		{
			$r = new Request($engine, $this->name, 'login');
			$icon = new PageElement('image', array(
					'stock' => 'login'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('Login')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'label' => $link));
			$r = new Request($engine, $this->name, 'register');
			$icon = new PageElement('image', array(
					'stock' => 'register'));
			$link = new PageElement('link', array(
					'request' => $r,
					'text' => _('Register')));
			if($this->can_register())
				$ret[] = new PageElement('row', array(
						'icon' => $icon,
						'label' => $link));
		}
		else
		{
			$r = new Request($engine, $this->name, 'admin');
			$icon = new PageElement('image', array(
					'stock' => 'admin'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('Administration')));
			if($cred->isAdmin())
				$ret[] = new PageElement('row', array(
						'icon' => $icon,
						'label' => $link));
			$r = new Request($engine, $this->name, 'display');
			$icon = new PageElement('image', array(
					'stock' => 'user'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('My content')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'label' => $link));
			$r = new Request($engine, $this->name, 'logout');
			$icon = new PageElement('image', array(
					'stock' => 'logout'));
			$link = new PageElement('link', array('request' => $r,
					'text' => _('Logout')));
			$ret[] = new PageElement('row', array('icon' => $icon,
					'label' => $link));
		}
		return $ret;
	}


	//UserModule::admin
	protected function admin($engine, $request)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Permission denied')));
		$title = _('User administration');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//FIXME really implement
		return $page;
	}


	//UserModule::can_register
	protected function can_register()
	{
		global $config;

		return $config->getVariable('module::user', 'register') == 1;
	}


	//UserModule::_default
	protected function _default($engine, $request = FALSE)
	{
		$cred = $engine->getCredentials();

		if($request !== FALSE && ($id = $request->getId()) !== FALSE)
			return $this->display($engine, $request);
		//FIXME add content?
		$title = ($cred->getUserId() != 0) ? _('User homepage')
			: _('Site menu');
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$view = $page->append('iconview');
		$actions = $this->actions($engine, $request);
		if(is_array($actions))
			foreach($actions as $a)
				$view->appendElement($a);
		return $page;
	}


	//UserModule::display
	protected function display($engine, $request)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		$view = $page->append('iconview');
		if(($uid = $request->getId()) !== FALSE)
			//FIXME verify the request's title if set
			$title = _('Content from ').$uid;
		else if(($uid = $cred->getUserId()) != 0)
		{
			$title = _('My content');
			$r = new Request($engine, $this->name);
			$page->append('link', array('stock' => 'back',
					'request' => $r,
					'text' => _('Back to my homepage')));
		}
		else
			return $this->login($engine, new Request);
		$page->setProperty('title', $title);
		$page->prepend('title', array('stock' => $this->name,
				'text' => $title));
		//FIXME request content from all modules
		$r = new Request($engine, $this->name, 'logout');
		$link = new PageElement('link', array('text' => _('Logout'),
				'request' => $r));
		$icon = new PageElement('image', array('stock' => 'logout'));
		$view->append('row', array('icon' => $icon, 'label' => $link));
		return $page;
	}


	//UserModule::login
	protected function login($engine, $request)
	{
		$page = new Page;
		$page->append('title', array('stock' => 'login',
					'text' => _('User login')));
		//process login
		$error = $this->_login($engine, $request);
		//login successful
		if($error === FALSE)
		{
			$r = new Request($engine);
			$page->setProperty('location', $engine->getUrl($r));
			$page->setProperty('refresh', 30);
			$box = $page->append('vbox');
			$text = _('Logging in progress, please wait...');
			$box->append('label', array('text' => $text));
			$box = $box->append('hbox');
			$text = 'If you are not redirected within 30 seconds,'
				.' please ';
			$box->append('label', array('text' => $text));
			$box->append('link', array('text' => 'click here',
						'request' => $r));
			$box->append('label', array('text' => '.'));
			return $page;
		}
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
						'text' => $error));
		$r = new Request($engine, $this->name, 'login');
		$form = $page->append('form', array('request' => $r));
		$entry = $form->append('entry', array(
					'name' => 'username',
					'text' => _('Username: '),
					'value' => $request->getParameter(
						'username')));
		$entry = $form->append('entry', array(
					'hidden' => TRUE,
					'name' => 'password',
					'text' => _('Password: ')));
		$form->append('button', array('text' => _('Cancel'),
					'stock' => 'cancel',
					'request' => new Request($engine)));
		$button = $form->append('button', array('type' => 'submit',
					'stock' => 'login',
					'text' => _('Login')));
		return $page;
	}

	private function _login($engine, $request)
	{
		$db = $engine->getDatabase();

		if(($username = $request->getParameter('username')) === FALSE
				|| ($password = $request->getParameter(
						'password')) === FALSE)
			return TRUE;
		if($engine->isIdempotent($request) !== FALSE)
			return _('The request expired or is invalid');
		//FIXME first obtain the password and apply salt if necessary
		$res = $db->query($engine, $this->query_login, array(
					'username' => $username,
					'password' => md5($password)));
		if($res === FALSE || count($res) != 1)
			return _('Invalid username or password');
		$res = $res[0];
		$cred = new AuthCredentials($res['user_id'], $res['username'],
				$res['group_id'], $res['admin'] == 1);
		if($engine->setCredentials($cred) !== TRUE)
			return _('Invalid username or password');
		return FALSE;
	}


	//UserModule::logout
	protected function logout($engine, $request)
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
			$r = new Request($engine);
			$page->append('link', array('stock' => 'back',
					'request' => $r,
					'text' => _('Back to the site')));
			return $page;
		}
		$r = new Request($engine, $this->name, 'logout');
		if($engine->isIdempotent($request))
		{
			//FIXME make it a question dialog
			$form = $page->append('form', array(
						'request' => $r));
			$vbox = $form->append('vbox');
			$vbox->append('label', array(
				'text' => _('Do you really want to logout?')));
			$r = new Request($engine, $this->name);
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


	//UserModule::menu
	protected function menu($engine, $request)
	{
		$cred = $engine->getCredentials();

		//FIXME set links and icons
		$menu = new PageElement('menuitem', array('text' => 'User'));
		if($cred->getUserId() == 0)
		{
			$menu->append('menuitem', array('text' => 'Login'));
			$menu->append('menuitem', array('text' => 'Register'));
		}
		else
		{
			if($cred->isAdmin())
				$menu->append('menuitem', array(
						'text' => 'Administration'));
			$menu->append('menuitem', array('text' => 'Logout'));
		}
		return $menu;
	}


	//UserModule::register
	protected function register($engine, $request)
	{
		$cred = $engine->getCredentials();
		$error = TRUE;

		if($cred->getUserId() != 0)
			//already registered and logged in
			return $this->display($engine, new Request);
		//process registration
		if(!$this->can_register())
			$error = _('Registering is not allowed');
		else if(!$engine->isIdempotent($request))
			$error = $this->_register_process($engine, $request);
		//registration successful
		if($error === FALSE)
			return $this->_register_success($engine, $request);
		return $this->_register_form($engine, $request, $error);
	}

	private function _register_form($engine, $request, $error)
	{
		$page = new Page(array('title' => _('User registration')));
		$page->append('title', array('stock' => $this->name,
				'text' => _('User registration')));
		if(is_string($error))
			$page->append('dialog', array('type' => 'error',
				'text' => $error));
		$r = new Request($engine, $this->name, 'register');
		$form = $page->append('form', array('request' => $r));
		$username = $request->getParameter('username');
		$email = $request->getParameter('email');
		$form->append('entry', array('text' => _('Username: '),
			'name' => 'username', 'value' => $username));
		$form->append('entry', array('text' => _('e-mail address: '),
			'name' => 'email', 'value' => $email));
		$form->append('button', array('stock' => 'cancel',
			'text' => _('Cancel'),
			'request' => new Request($engine, $this->name)));
		$form->append('button', array('stock' => 'register',
			'type' => 'submit', 'text' => _('Register')));
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
		if(($user = User::register($engine, $username, FALSE, $email,
					FALSE, $error)) === FALSE)
			$ret .= $error;
		return strlen($ret) ? $ret : FALSE;
	}

	private function _register_success($engine, $request)
	{
		$page = new Page(array('title' => _('User registration')));
		$page->append('title', array('stock' => $this->name,
				'text' => _('User registration')));
		$page->append('dialog', array('type' => 'info',
				'text' => _("You should receive an e-mail shortly with your password, along with a confirmation key.\n
Thank you for registering!")));
		$page->append('link', array('stock' => 'back',
			'text' => _('Back to the site'),
			'request' => new Request($engine)));
		return $page;
	}


	//UserModule::validate
	protected function validate($engine, $request)
	{
		$cred = $engine->getCredentials();
		$error = TRUE;
		$uid = $request->getId();
		$token = $request->getParameter('token');

		if($cred->getUserId() != 0)
			//already registered and logged in
			return $this->display($engine, new Request);
		$page = new Page(array('title' => _('Account confirmation')));
		$page->append('title', array('stock' => $this->name,
				'text' => _('Account confirmation')));
		if(!$this->can_register())
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
			$r = new Request($engine, $this->name);
			$box->append('link', array('stock' => 'login',
					'request' => $r,
					'text' => _('Login')));
		}
		$r = new Request($engine);
		$box->append('link', array('stock' => 'back', 'request' => $r,
			'text' => _('Back to the site')));
		return $page;
	}


	//UserModule::widget
	protected function widget($engine, $request)
	{
		$cred = $engine->getCredentials();

		if($cred->getUserId() == 0)
		{
			$r = new Request($engine, $this->name, 'login');
			$form = new PageElement('form', array('request' => $r));
			$entry = $form->append('entry', array(
				'name' => 'username',
				'text' => _('Username: '),
				'value' => $request->getParameter(
					'username')));
			$entry = $form->append('entry', array(
				'hidden' => TRUE,
				'name' => 'password',
				'text' => _('Password: ')));
			$form->append('button', array('type' => 'submit',
				'stock' => 'login',
				'text' => _('Login')));
			return $form;
		}
		$box = new PageElement('vbox');
		$r = new Request($engine, $this->name);
		$box->append('button', array('stock' => 'home',
				'request' => $r,
				'text' => _('Homepage')));
		$r = new Request($engine, $this->name, 'display');
		$box->append('button', array('stock' => 'user',
				'request' => $r,
				'text' => _('My content')));
		$r = new Request($engine, $this->name, 'logout');
		$box->append('button', array('stock' => 'logout',
				'request' => $r,
				'text' => _('Logout')));
		return $box;
	}


	//private
	//properties
	//queries
	private $query_login = "SELECT user_id, group_id, username, admin
		FROM daportal_user
		WHERE username=:username AND password=:password
		AND enabled='1'";
}

?>
