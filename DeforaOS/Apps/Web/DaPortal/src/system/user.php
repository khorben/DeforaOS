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



//User
class User
{
	//public
	//methods
	//essential
	public function __construct($uid, $username = FALSE)
	{
		//FIXME really implement
		$this->user_id = $uid;
	}


	//accessors
	//User::getUserId
	public function getUserId()
	{
		return $this->user_id;
	}


	//User::isAdmin
	public function isAdmin()
	{
		return $this->admin;
	}


	//User::isEnabled
	public function isEnabled()
	{
		return $this->enabled;
	}


	//User::setPassword
	public function setPassword(&$engine, $password)
	{
		$db = $engine->getDatabase();

		$res = $db->query($engine, $this->query_set_password, array(
					'user_id' => $this->user_id,
					'password' => md5($password)));
		return ($res !== FALSE);
	}


	//static
	//useful
	//User::register
	static public function register(&$engine, $username, $email,
			$enabled = FALSE)
	{
		$db = $engine->getDatabase();

		$res = $db->query($engine, User::$query_register,
				array('username' => $username,
					'email' => $email,
					'enabled' => $enabled ? 1 : 0));
		if($res === FALSE || ($uid = $db->getLastId($engine,
						'daportal_user', 'user_id'))
				=== FALSE)
			return FALSE;
		$user = new User($uid);
		if($user->getUserId() === FALSE)
			return FALSE;
		return $user;
	}


	//private
	//properties
	private $user_id = FALSE;
	private $username = FALSE;
	private $group_id = FALSE;
	private $enabled = FALSE;
	private $admin = FALSE;

	//queries
	private $query_set_password = 'UPDATE daportal_user
		SET password=:password
		WHERE user_id=:user_id';
	//static
	static private $query_register = 'INSERT INTO daportal_user
		(username, email, enabled)
		VALUES (:username, :email, :enabled)';
}

?>
