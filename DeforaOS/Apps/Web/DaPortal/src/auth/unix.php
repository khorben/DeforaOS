<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//UnixAuth
require_once('./system/auth.php');
class UnixAuth extends Auth
{
	//protected
	//methods
	//UnixAuth::match
	protected function match(&$engine)
	{
		return 1;
	}


	//UnixAuth::attach
	protected function attach(&$engine)
	{
		if(($db = $engine->getDatabase()) === FALSE)
			return TRUE;
		$query = 'SELECT user_id, admin'
			.' FROM daportal_user'
			." WHERE enabled='1' AND username=:username";
		$uid = posix_getuid();
		$pw = posix_getpwuid($uid);
		$args = array('username' => $pw['name']);
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return TRUE;
		$cred = $this->getCredentials();
		$cred->setUserId($res[0]['user_id'], $res[0]['admin']);
		$this->setCredentials($engine, $cred);
		return TRUE;
	}
}

?>
