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



require_once('./system/mail.php');


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


	//User::setEnabled
	public function setEnabled(&$engine, $enabled)
	{
		$db = $engine->getDatabase();

		$res = $db->query($engine, $this->query_set_enabled, array(
					'user_id' => $this->user_id,
					'enabled' => $enabled ? 1 : 0));
		return ($res !== FALSE);
	}


	//static
	//useful
	//User::password_new
	static public function password_new()
	{
		$string = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
			.'0123456789';
		$password = '';
		for($i = 0; $i < 8; $i++)
			$password .= $string[rand(0, strlen($string) - 1)];
		return $password;
	}


	//User::register
	static public function register(&$engine, $username, $email,
			$enabled = FALSE, &$error = FALSE)
	{
		$db = $engine->getDatabase();
		$error = '';

		//FIXME really validate username
		if(!is_string($username) || strlen($username) == 0)
			$error .= _("The username is not valid\n");
		//FIXME really validate e-mail
		if(strchr($email, '@') === FALSE)
			$error .= _("The e-mail address is not valid\n");
		//FIXME verify that the username and e-mail are both unique
		if(strlen($error) > 0)
			return FALSE;
		if($db->transactionBegin($engine) === FALSE)
		{
			$error = _('Could not register the user');
			return FALSE;
		}
		$res = $db->query($engine, User::$query_register,
				array('username' => $username,
					'email' => $email,
					'enabled' => $enabled ? 1 : 0));
		if($res === FALSE || ($uid = $db->getLastId($engine,
						'daportal_user', 'user_id'))
				=== FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not register the user');
			return FALSE;
		}
		$user = new User($uid);
		if($user->getUserId() === FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not register the user');
			return FALSE;
		}
		if($enabled === FALSE)
		{
			$password = User::password_new();
			$token = sha1(uniqid($password, TRUE));
			if($user->setPassword($engine, $password) === FALSE
				|| $db->query($engine,
						User::$query_register_token,
						array('user_id' => $uid,
						'token' => $token)) === FALSE)
			{
				$db->transactionRollback($engine);
				$error = _('Could not register the user');
				return FALSE;
			}
			//FIXME the request should be given as argument
			$r = new Request($engine, 'user', 'validate', $uid,
					FALSE, array('token' => $token));
			$subject = _('User registration'); //XXX add site title
			$text = _("Thank you for registering on this site.\n");
			$text .= _("\nYour password is: ").$password."\n";
			$text .= _("\nPlease click on the following link to enable your account:\n");
			$text .= $engine->getUrl($r)."\n";
			$content = new PageElement('label', array(
				'text' => $text));
			Mail::send($engine, FALSE, $email, $subject, $content);
		}
		$db->transactionCommit($engine);
		$error = FALSE;
		return $user;
	}


	//User::validate
	static public function validate($engine, $uid, $token, &$error = FALSE)
	{
		$db = $engine->getDatabase();
		$error = '';

		if($uid === FALSE || !is_numeric($uid))
			$error .= _("Unknown user ID\n");
		if($token === FALSE)
			$error .= _("The token must be specified\n");
		if(strlen($error) > 0)
			return FALSE;
		$timestamp = 0; //FIXME really implement
		if($db->query($engine, User::$query_register_cleanup, array(
					'timestamp' => $timestamp)) === FALSE)
		{
			$error = _("Could not validate the user\n");
			return FALSE;
		}
		$res = $db->query($engine, User::$query_register_validate,
				array('user_id' => $uid, 'token' => $token));
		if($res === FALSE || count($res) != 1)
		{
			$error = _('Could not validate the user');
			return FALSE;
		}
		$res = $res[0];
		if($db->transactionBegin($engine) === FALSE)
		{
			$error = _('Could not validate the user');
			return FALSE;
		}
		$query = User::$query_register_delete;
		if($db->query($engine, $query, array('user_register_id'
				=> $res['user_register_id']))
				=== FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not validate the user');
			return FALSE;
		}
		if($db->query($engine, User::$query_register_delete, array(
				'user_register_id' => $res['user_register_id']))
				=== FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not validate the user');
			return FALSE;
		}
		$user = new User($res['user_id']);
		if($user->setEnabled($engine, TRUE) === FALSE
				|| $db->transactionCommit($engine) === FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not enable the user');
			return FALSE;
		}
		return new User($res['user_id']);
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
	private $query_set_enabled = "UPDATE daportal_user
		SET enabled=:enabled
		WHERE user_id=:user_id";
	//static
	static private $query_register = 'INSERT INTO daportal_user
		(username, email, enabled)
		VALUES (:username, :email, :enabled)';
	static private $query_register_token = 'INSERT INTO daportal_user_register
		(user_id, token)
		VALUES (:user_id, :token)';
	static private $query_register_cleanup = 'DELETE FROM daportal_user_register
		WHERE timestamp <= :timestamp';
	static private $query_register_delete = 'DELETE FROM daportal_user_register
		WHERE user_register_id=:user_register_id';
	static private $query_register_validate = 'SELECT user_register_id,
		daportal_user.user_id AS user_id, username
		FROM daportal_user, daportal_user_register
		WHERE daportal_user.user_id=daportal_user_register.user_id
		AND token=:token';
}

?>
