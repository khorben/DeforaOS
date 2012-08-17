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



require_once('./system/auth.php');


//UnixAuth
class UnixAuth extends Auth
{
	//protected
	//methods
	//UnixAuth::match
	protected function match($engine)
	{
		return 1;
	}


	//UnixAuth::attach
	protected function attach($engine)
	{
		$uid = posix_getuid();
		$pw = posix_getpwuid($uid);
		if(($user = User::lookup($engine, $pw['name'])) === FALSE)
			return TRUE;
		$cred = new AuthCredentials($user->getUserId(),
				$user->getUsername(), $user->getGroupId(),
				$user->isAdmin());
		$this->setCredentials($engine, $cred);
		return TRUE;
	}
}

?>
