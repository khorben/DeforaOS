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


//SessionAuth
class SessionAuth extends Auth
{
	//protected
	//methods
	//SessionAuth::match
	protected function match(&$engine)
	{
		return session_start() ? 100 : 0;
	}


	//SessionAuth::attach
	protected function attach(&$engine)
	{
	}


	//public
	//accessors
	//SessionAuth::getCredentials
	public function getCredentials(&$engine)
	{
		@session_start();
		if(!isset($_SESSION['auth']['uid']))
			return parent::getCredentials();
		if($_SESSION['auth']['uid'] == 0)
			return parent::getCredentials();
		if(($db = $engine->getDatabase()) === FALSE)
			return parent::getCredentials();
		$uid = $_SESSION['auth']['uid'];
		$args = array('uid' => $uid);
		if(($res = $db->query($engine, $this->query_credentials,
						$args)) === FALSE
				|| count($res) != 1)
			return parent::getCredentials();
		$res = $res[0];
		//FIXME check if $admin is set properly (eg not always)
		$cred = new AuthCredentials($res['user_id'], $res['username'],
				$res['group_id'], $res['admin'] == 1);
		parent::setCredentials($cred);
		return parent::getCredentials();
	}


	//SessionAuth::setCredentials
	public function setCredentials($credentials)
	{
		@session_start();
		$_SESSION['auth']['uid'] = $credentials->getUserId();
		return parent::setCredentials($credentials);
	}


	//private
	//properties
	//queries
	private $query_credentials = "SELECT user_id, group_id, username, admin
		FROM daportal_user
		WHERE user_id=:uid AND enabled='1'";
}

?>
