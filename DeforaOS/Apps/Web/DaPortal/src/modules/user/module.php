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


//UserModule
class UserModule extends Module
{
	//UserModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'display':
			case 'login':
			case 'logout':
				return $this->$action($engine, $request);
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
	//methods
	//UserModule::_default
	protected function _default($engine, $request)
	{
		if(($id = $request->getId()) !== FALSE)
			return $this->display($engine, $request);
		$cred = $engine->getCredentials();
		if($cred->getUserId() != 0)
			return $this->display($engine, new Request);
		return $this->login($engine, $request);
	}


	//UserModule::display
	protected function display($engine, $request)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		if(($uid = $request->getId()) !== FALSE)
			$title = 'User '.$uid;
		else
		{
			$uid = $cred->getUserId();
			$title = 'User homepage';
		}
		//FIXME verify the request's title if set
		$page->append('title', array('text' => $title));
		//FIXME really implement
		$r = new Request($engine, 'user', 'logout');
		$page->append('link', array('text' => 'Logout',
					'request' => $r));
		return $page;
	}


	//UserModule::login
	protected function login($engine, $request)
	{
		$error = TRUE;

		$page = new Page;
		$page->append('title', array('text' => 'User login'));
		//process login
		if($engine->isIdempotent($engine) === FALSE
				&& ($u = $request->getParameter('username'))
				&& ($p = $request->getParameter('password')))
			$error = $this->_login($engine, $u, $p);
		//login successful
		if($error === FALSE)
		{
			$r = new Request($engine);
			$page->setProperty('location', $engine->getUrl($r));
			$page->setProperty('refresh', 30);
			$box = $page->append('vbox');
			$text = 'Logging in progress, please wait...';
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
		$r = new Request($engine, 'user', 'login');
		$form = $page->append('form', array('request' => $r));
		$entry = $form->append('entry', array(
					'name' => 'username',
					'text' => 'Username: ',
					'value' => $request->getParameter(
						'username')));
		$entry = $form->append('entry', array(
					'hidden' => TRUE,
					'name' => 'password',
					'text' => 'Password: '));
		$button = $form->append('button', array('type' => 'submit',
					'text' => 'Login'));
		return $page;
	}

	private function _login($engine, $username, $password)
	{
		$db = $engine->getDatabase();

		//FIXME first obtain the password and apply salt if necessary
		$res = $db->query($engine, $this->query_login, array(
					'username' => $username,
					'password' => md5($password)));
		if($res === FALSE || count($res) != 1)
			return 'Invalid username or password';
		$res = $res[0];
		$cred = new AuthCredentials($res['user_id'], $res['username'],
				$res['group_id'], $res['admin'] == 1);
		if($engine->setCredentials($cred) !== TRUE)
			return 'Invalid username or password';
		return FALSE;
	}


	//UserModule::logout
	protected function logout($engine, $request)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		$page->append('title', array('text' => 'User logout'));
		if($cred->getUserId() == 0)
		{
			$text = 'You were logged out successfully';
			$page->append('dialog', array('type' => 'info',
						'text' => $text));
			$r = new Request($engine);
			$page->append('link', array('request' => $r,
						'text' => 'Back to the site'));
			return $page;
		}
		//process logout
		$r = new Request($engine, 'user', 'logout');
		$page->setProperty('location', $engine->getUrl($r));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$text = 'Logging out, please wait...';
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


	//private
	//properties
	//queries
	private $query_login = "SELECT user_id, group_id, username, admin
		FROM daportal_user
		WHERE username=:username AND password=:password
		AND enabled='1'";
}

?>
