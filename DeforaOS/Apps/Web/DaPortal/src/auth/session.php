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



//SessionAuth
require_once('./system/auth.php');
class SessionAuth extends Auth
{
	//protected
	//methods
	//SessionAuth::match
	protected function match(&$engine)
	{
		return 0;
	}


	//SessionAuth::attach
	protected function attach(&$engine)
	{
		session_start();
		if(($db = $engine->getDatabase()) === FALSE)
			return TRUE;
		$uid = (isset($_SESSION['auth']['uid']))
			? $_SESSION['auth']['uid'] : 0;
		$query = 'SELECT admin FROM daportal_user'
			." WHERE enabled='1' AND user_id=:uid";
		$args = array('uid' => $uid);
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return TRUE;
		$cred = $this->getCredentials();
		$cred->setUserId($uid, $res[0]['admin']);
		$cred->setGroupId($gid);
		parent::setCredentials($cred);
		return TRUE;
	}


	//private
	//methods
	protected function setCredentials($credentials)
	{
		$_SESSION['auth']['uid'] = $credentials->getUserId();
		$_SESSION['auth']['gid'] = $credentials->getGroupId();
		parent::setCredentials($credentials);
	}
}

?>
