<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
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



//HttpAuth
require_once('./system/auth.php');
class HttpAuth extends Auth
{
	//protected
	//methods
	//HttpAuth::match
	protected function match(&$engine)
	{
		if(isset($_SERVER['PHP_AUTH_USER']))
			return 100;
		return 0;
	}


	//HttpAuth::attach
	protected function attach(&$engine)
	{
		if(!isset($_SERVER['PHP_AUTH_USER']))
		{
			//FIXME let the realm be configureable
			header('WWW-Authenticate: Basic realm="DaPortal"');
			header('HTTP/1.0 401 Unauthorized');
			return TRUE;
		}
		if(($db = $engine->getDatabase()) === FALSE)
			return TRUE;
		$username = $_SERVER['PHP_AUTH_USER'];
		$password = isset($_SERVER['PHP_AUTH_PW'])
			? md5($_SERVER['PHP_AUTH_PW']) : '';
		$query = 'SELECT user_id, admin FROM daportal_user'
			." WHERE enabled='1' AND username=:username"
			." AND password=:password";
		$args = array('username' => $username, 'password' => $password);
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return TRUE;
		$cred = $this->getCredentials();
		$cred->setUserId($res[0]['user_id'], $res[0]['admin']);
		parent::setCredentials($cred);
		return TRUE;
	}
}

?>
