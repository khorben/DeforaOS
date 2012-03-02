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
		if(!isset($_SERVER['SCRIPT_NAME'])
				|| !isset($_SERVER['HTTP_HOST']))
			return 0;
		if($this->match_score !== FALSE)
			return $this->match_score;
		@ini_set('session.use_only_cookies', 1);
		@ini_set('session.use_trans_sid', 0);
		$params = session_get_cookie_params();
		//XXX probably not always as strict as it could be
		$params['path'] = dirname($_SERVER['SCRIPT_NAME']);
		$params['domain'] = $_SERVER['HTTP_HOST'];
		if(isset($_SERVER['HTTPS']))
			$params['secure'] = 1;
	       	//XXX we may have to set it to 0 later
		$params['httponly'] = 1;
		session_set_cookie_params($params['lifetime'], $params['path'],
				$params['domain'], $params['secure'],
				$params['httponly']);
		$this->match_score = @session_start() ? 100 : 0;
		return $this->match_score;
	}


	//SessionAuth::attach
	protected function attach(&$engine)
	{
		$this->match($engine);
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
		parent::setCredentials($engine, $cred);
		return parent::getCredentials();
	}


	//SessionAuth::setCredentials
	public function setCredentials(&$engine, $credentials)
	{
		if(session_regenerate_id(TRUE) !== TRUE)
			$engine->log('LOG_WARNING',
					'Could not regenerate the session');
		$_SESSION['auth']['uid'] = $credentials->getUserId();
		return parent::setCredentials($engine, $credentials);
	}


	//private
	//properties
	private $match_score = FALSE;

	//queries
	private $query_credentials = "SELECT user_id, group_id, username, admin
		FROM daportal_user
		WHERE user_id=:uid AND enabled='1'";
}

?>
