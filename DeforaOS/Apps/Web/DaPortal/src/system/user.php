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
	public function __construct(&$engine, $uid, $username = FALSE)
	{
		$db = $engine->getDatabase();
		$query = $username ? $this->query_get_by_id_username
			: $this->query_get_by_id;

		$this->user_id = 0;
		$this->username = 'anonymous';
		$this->group_id = 0;
		$this->groupname = 'nogroup';
		$this->admin = FALSE;
		if(($res = $db->query($engine, $query, array(
					'user_id' => $uid,
					'username' => $username))) === FALSE
				|| count($res) != 1
				|| ($username !== FALSE
				&& $res[0]['username'] != $username))
			return;
		$res = $res[0];
		$this->user_id = $res['id'];
		$this->username = $res['username'];
		$this->admin = $res['admin'] ? TRUE : FALSE;
	}


	//accessors
	//User::getUserId
	public function getUserId()
	{
		return $this->user_id;
	}


	//User::getUsername
	public function getUsername()
	{
		return $this->username;
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
	static public function register(&$engine, $username, $password, $email,
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
		$user = new User($engine, $uid);
		if($user->getUserId() === FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not register the user');
			return FALSE;
		}
		if($enabled === FALSE)
		{
			//let the user confirm registration
			if($password === FALSE)
				//generate a random password
				$password = User::password_new();
			//generate a token
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
			//send an e-mail for confirmation
			//FIXME the request should be given as argument
			$r = new Request($engine, 'user', 'validate', $uid,
					FALSE, array('token' => $token));
			$subject = _('User registration'); //XXX add site title
			$text = _("Thank you for registering on this site.\n");
			//FIXME do not send the password if already known
			$text .= _("\nYour password is: ").$password."\n";
			$text .= _("\nPlease click on the following link to enable your account:\n");
			$text .= $engine->getUrl($r)."\n";
			$text .= _("Please note that this link will expire in 7 days.\n");
			$content = new PageElement('label', array(
				'text' => $text));
			Mail::send($engine, FALSE, $email, $subject, $content);
		}
		$db->transactionCommit($engine);
		$error = '';
		return TRUE;
	}


	//User::reset
	static public function reset(&$engine, $username, $email,
			&$error = FALSE)
	{
		$db = $engine->getDatabase();
		$error = '';

		$timestamp = date('Y-m-d H:i:s', time() - 86400); //one day
		//verify the username and e-mail address
		$res = $db->query($engine, User::$query_reset_validate,
			array('username' => $username,
				'email' => $email));
		if($res === FALSE || count($res) != 1)
		{
			//XXX consider silently failing (to avoid bruteforcing)
			$error = _('Could not reset the password');
			return FALSE;
		}
		$res = $res[0];
		$uid = $res['user_id'];
		//generate a token
		$token = sha1(uniqid($uid.$username.$email, TRUE));
		$res = $db->query($engine, User::$query_reset_token,
			array('user_id' => $uid, 'token' => $token));
		if($res === FALSE)
		{
			$error = _('Could not reset the password');
			return FALSE;
		}
		//send an e-mail with the token
		//FIXME the request should be given as argument
		$r = new Request($engine, 'user', 'reset', $uid,
			FALSE, array('token' => $token));
		$subject = _('Password reset'); //XXX add site title
		$text = _("Someone, hopefully you, has requested a password reset on your account.\n");
		$text .= _("\nPlease click on the following link to reset your password:\n");
		$text .= $engine->getUrl($r)."\n";
		$text .= _("Please note that this link will expire in 24 hours.\n");
		$content = new PageElement('label', array('text' => $text));
		Mail::send($engine, FALSE, $email, $subject, $content);
		return TRUE;
	}


	//User::reset_password
	static function reset_password(&$engine, $uid, $password, $token,
			&$error = FALSE)
	{
		$db = $engine->getDatabase();
		$error = _('Could not reset the password');

		if($db->transactionBegin($engine) === FALSE)
			return FALSE;
		$timestamp = date('Y-m-d H:i:s', time() - 86400); //one day
		if($db->query($engine, User::$query_reset_cleanup, array(
					'timestamp' => $timestamp)) === FALSE)
		{
			$db->transactionRollback($engine);
			return FALSE;
		}
		//lookup the token
		$res = $db->query($engine, User::$query_reset_validate_token,
				array('user_id' => $uid, 'token' => $token));
		if($res === FALSE || count($res) != 1)
		{
			$db->transactionRollback($engine);
			return FALSE;
		}
		$user = new User($engine, $uid);
		if($user->setPassword($engine, $password) === FALSE)
		{
			$db->transactionRollback($engine);
			return FALSE;
		}
		if($db->query($engine, User::$query_reset_delete, array(
					'user_id' => $uid, 'token' => $token))
				=== FALSE)
		{
			$db->transactionRollback($engine);
			return FALSE;
		}
		if($db->transactionCommit($engine) === FALSE)
			return FALSE;
		$error = '';
		return FALSE;
	}


	//User::validate
	static public function validate(&$engine, $uid, $token, &$error = FALSE)
	{
		$db = $engine->getDatabase();
		$error = '';

		if($uid === FALSE || !is_numeric($uid))
			$error .= _("Unknown user ID\n");
		if($token === FALSE)
			$error .= _("The token must be specified\n");
		if(strlen($error) > 0)
			return FALSE;
		$timestamp = date('Y-m-d H:i:s', time() - 604800); //one week
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
		$user = new User($engine, $res['user_id']);
		if($user->setEnabled($engine, TRUE) === FALSE
				|| $db->transactionCommit($engine) === FALSE)
		{
			$db->transactionRollback($engine);
			$error = _('Could not enable the user');
			return FALSE;
		}
		return new User($engine, $res['user_id']);
	}


	//private
	//properties
	private $user_id = FALSE;
	private $username = FALSE;
	private $group_id = FALSE;
	private $enabled = FALSE;
	private $admin = FALSE;

	//queries
	private $query_get_by_id = "SELECT user_id AS id, username,
		daportal_user.enabled AS enabled,
		daportal_user.group_id AS group_id, groupname, admin
		FROM daportal_user
		LEFT JOIN daportal_group
		ON daportal_user.group_id=daportal_group.group_id
		WHERE daportal_group.enabled='1'
		AND daportal_user.enabled='1'
		AND user_id=:user_id";
	private $query_get_by_id_username = "SELECT user_id AS id, username,
		daportal_user.enabled AS enabled,
		daportal_user.group_id AS group_id, groupname, admin
		FROM daportal_user
		LEFT JOIN daportal_group
		ON daportal_user.group_id=daportal_group.group_id
		WHERE daportal_group.enabled='1'
		AND daportal_user.enabled='1'
		AND user_id=:user_id
		AND username=:username";
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
	static private $query_reset_cleanup = 'DELETE FROM daportal_user_reset
		WHERE timestamp <= :timestamp';
	static private $query_reset_delete = 'DELETE FROM daportal_user_reset
		WHERE user_id=:user_id AND token=:token';
	static private $query_reset_token = 'INSERT INTO daportal_user_reset
		(user_id, token)
		VALUES (:user_id, :token)';
	static private $query_reset_validate = "SELECT user_id
		FROM daportal_user
		WHERE enabled='1' AND username=:username AND email=:email";
	static private $query_reset_validate_token = "SELECT
		daportal_user.user_id AS user_id, username
		FROM daportal_user, daportal_user_reset
		WHERE daportal_user.user_id=daportal_user_reset.user_id
		AND enabled='1' AND daportal_user.user_id=:user_id
		AND token=:token";
}

?>
