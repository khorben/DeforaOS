<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <pierre.pronchery@duekin.com>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//TODO:
//- use the browser capability to generate PKCS#12 itself?
//- secure key when exporting PKCS#12



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['CA_LIST'] = 'CA list';
$text['CACLIENT_LIST'] = 'Client list';
$text['CASERVER_LIST'] = 'Server list';
$text['COMMON_NAME'] = 'Common Name';
$text['CN'] = 'CN';
$text['COUNTRY'] = 'Country';
$text['EMAIL'] = 'e-mail';
$text['EXPORT'] = 'Export';
$text['KEY'] = 'Key';
$text['IMPORT'] = 'Import';
$text['INCLUDE_PRIVATE_KEY'] = 'Include private key';
$text['LOCALITY'] = 'Locality';
$text['NEW_CA'] = 'New CA';
$text['NEW_CACLIENT_FOR'] = 'New client for';
$text['NEW_CASERVER_FOR'] = 'New server for';
$text['ORGANIZATION'] = 'Organization';
$text['PARENT'] = 'Parent';
$text['PARENT_CA'] = 'Parent CA';
$text['PKI'] = 'PKI';
$text['PKI_ADMINISTRATION'] = 'PKI administration';
$text['PUBLIC_KEY_INFRASTRUCTURE'] = 'Public key infrastructure';
$text['RENEW'] = 'Renew';
$text['REVOKE'] = 'Revoke';
$text['SECTION'] = 'Section';
$text['SELF_SIGNED'] = 'Self-signed';
$text['SETTINGS'] = 'Settings';
$text['STATE'] = 'State';
$text['UNIT'] = 'Unit';
global $lang;
if($lang == 'fr')
{
	$text['CA_LIST'] = 'Liste des CA';
	$text['CACLIENT_LIST'] = 'Liste des clients';
	$text['CASERVER_LIST'] = 'Liste des serveurs';
	$text['COMMON_NAME'] = 'Nom usuel (CN)';
	$text['COUNTRY'] = 'Pays';
	$text['LOCALITY'] = 'Ville';
	$text['NEW_CA'] = 'Nouveau CA';
	$text['ORGANIZATION'] = 'Organisation';
	$text['PARENT_CA'] = 'CA parent';
	$text['PKI_ADMINISTRATION'] = 'Administration du PKI';
	$text['SELF_SIGNED'] = 'Auto-signé';
	$text['SETTINGS'] = 'Paramètres';
	$text['STATE'] = 'Région';
}
_lang($text);


//PkiModule
class PkiModule extends Module
{
	//public
	//methods
	//useful
	//PkiModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		global $user_id;

		//FIXME this should be configurable
		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'ca_export':
			case 'ca_import':
			case 'ca_insert':
			case 'caclient_export':
			case 'caclient_insert':
			case 'caserver_export':
			case 'caserver_insert':
			case 'config_update':
			case 'delete':
			case 'disable':
			case 'display':
			case 'enable':
			case 'renew':
			case 'revoke':
			case 'system':
				return $this->$action($args);
			default:
				return $this->_default($args);
		}
		return FALSE;
	}


	//private
	//PkiModule::ca_get
	//get a CA
	private function _ca_get($id)
	{
		$parent = _sql_array('SELECT ca_id AS id, title'
				.' FROM daportal_ca, daportal_content'
				.' WHERE daportal_ca.ca_id'
				.'=daportal_content.content_id'
				." AND enabled='1' AND ca_id='$id'");
		if(!is_array($parent) || count($parent) != 1)
			return FALSE;
		return $parent[0];
	}


	//PkiModule::_exec
	//helper function for exec()
	private function _exec($cmd, &$output)
	{
		$ret = 1;
		_info($cmd);
		exec($cmd, $output, $ret);
		return $ret;
	}


	//PkiModule::_insert_cleanup
	//cleanup a CA or client files when an error occurs
	private function _insert_cleanup($cadir, $dirs = FALSE, $files = FALSE)
	{
		if(is_array($files))
			foreach($files as $f)
				@unlink($cadir.'/'.$f);
		if(is_array($dirs))
			foreach($dirs as $d)
				@rmdir($cadir.'/'.$d);
		@rmdir($cadir);
	}


	//PkiModule::_mkdir
	//variant of mkdir() with optional recursivity
	private function _mkdir($pathname, $mode = 0777, $recursive = FALSE)
	{
		if(!$recursive)
			return mkdir($pathname, $mode, $recursive);
		$dir = '';
		$dirs = explode('/', $pathname);
		foreach($dirs as $d)
		{
			$dir.=$d.'/';
			if(is_dir($dir))
				continue;
			if(mkdir($dir, $mode) != TRUE)
				return FALSE;
		}
		return TRUE;
	}


	//PkiModule::_ssl_config
	//creates an openssl.cnf file for a new CA
	private function _ssl_config($home, $cadir, $ca)
	{
		if(($fp = fopen($cadir.'/openssl.cnf', 'w')) == FALSE)
			return FALSE;
		ob_start(array($this, '_config_true'));
		include('./modules/pki/openssl.cnf.tpl');
		fwrite($fp, ob_get_contents());
		ob_end_clean();
		if(fclose($fp) != TRUE)
			return FALSE;
		return TRUE;
	}

	private function _config_true($void)
	{
		return TRUE;
	}


	//PkiModule::_subject_from_ca
	//returns the subject line for a given CA specification
	private function _subject_from_ca($ca)
	{
		//FIXME should escape slashes?
		$ret = '';
		$fields = array('country' => 'C', 'state' => 'ST',
				'locality' => 'L', 'organization' => 'O',
				'section' => 'OU', 'cn' => 'CN',
				'email' => 'emailAddress');
		foreach($fields as $k => $v)
			if(isset($ca[$k]) && strlen($ca[$k]) > 0)
				$ret .= '/'.$fields[$k].'='.$ca[$k];
		return $ret;
	}


//PkiModule::admin
protected function admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title pki">'._html_safe(PKI_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('pki')))
	{
		print('<h2 class="title settings"> '._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'pki';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title ca"> '._html_safe(CA_LIST)."</h2>\n");
	$sql = 'SELECT ca_id AS id, title, enabled, country, state, locality'
		.', organization, section, cn, email'
		.' FROM daportal_ca, daportal_content'
		.' WHERE daportal_ca.ca_id=daportal_content.content_id';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list CAs');
	$classes = array('enabled' => ENABLED, 'country' => COUNTRY,
			'state' => STATE, 'locality' => LOCALITY,
			'organization' => ORGANIZATION, 'section' => SECTION,
			'cn' => CN, 'email' => EMAIL);
	$keys = array_keys($classes);
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/ca.png';
		$res[$i]['thumbnail'] = 'icons/48x48/ca.png';
		$res[$i]['module'] = 'pki';
		$res[$i]['apply_module'] = 'pki';
		$res[$i]['action'] = 'display';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		foreach($keys as $k)
			$res[$i][$k] = _html_safe($res[$i][$k]);
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
			.$res[$i]['enabled'].'.png" alt="'
			.$res[$i]['enabled'].'" title="'
			.($res[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
		$res[$i]['email'] = '<a href="mailto:'.$res[$i]['email'].'">'
			.$res[$i]['email'].'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CA, 'class' => 'new',
			'link' => _module_link('pki', 'ca_insert'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => $classes, 'module' => 'pki',
				'action' => 'admin', 'toolbar' => $toolbar,
				'view' => 'details'));
	//XXX rename and move this function
	//server list
	$this->_display_ca_list_type(FALSE, 'caserver', CASERVER_LIST, '');
	//client list
	$this->_display_ca_list_type(FALSE, 'caclient', CACLIENT_LIST, '');
}


	//PkiModule::ca_export
	protected function ca_export($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return $this->_display_ca($args);
	}


	//PkiModule::ca_import
	protected function ca_import($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		return $this->_display_ca($args);
	}


	//PkiModule::ca_insert
	protected function ca_insert($args)
	{
		global $error;

		if(isset($error) && strlen($error))
			_error($error);
		$title = NEW_CA;
		$ca = array();
		$fields = array('title', 'country', 'state', 'locality',
				'organization', 'section', 'cn', 'email');
		foreach($fields as $f)
			if(isset($args[$f]))
				$ca[$f] = stripslashes($args[$f]);
		$parent = _sql_array('SELECT ca_id AS id, title'
				.' FROM daportal_ca, daportal_content'
				.' WHERE daportal_ca.ca_id=daportal_content.content_id'
				." AND enabled='1'");
		if(isset($args['parent']) && $this->_ca_get($args['parent'])
				!== FALSE)
			$parent_id = $args['parent'];
		include('./modules/pki/ca_update.tpl');
	}


	//PkiModule::caclient_export
	protected function caclient_export($args)
	{
		global $error;
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);

		if(isset($error) && strlen($error))
			_error($error);
		return $this->_display_type($args, 'caclient');
	}


	//PkiModule::caclient_insert
	protected function caclient_insert($args)
	{
		global $error;
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);

		if(isset($error) && strlen($error))
			_error($error);
		if(!isset($args['parent']) || ($parent = $this->_ca_get($args['parent'])) === FALSE)
			return _error(INVALID_ARGUMENT);
		$title = NEW_CACLIENT_FOR.' '.$parent['title'];
		$caclient = array();
		$fields = array('title', 'country', 'state', 'locality',
				'organization', 'section', 'cn', 'email');
		foreach($fields as $f)
			if(isset($args[$f]))
				$caclient[$f] = stripslashes($args[$f]);
		include('./modules/pki/caclient_update.tpl');
	}


	//PkiModule::caserver_export
	protected function caserver_export($args)
	{
		global $error;
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);

		if(isset($error) && strlen($error))
			_error($error);
		return $this->_display_type($args, 'caserver');
	}


	//PkiModule::caserver_insert
	protected function caserver_insert($args)
	{
		global $error;
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);

		if(isset($error) && strlen($error))
			_error($error);
		if(!isset($args['parent']) || !($parent = $this->_ca_get($args['parent'])))
			return _error(INVALID_ARGUMENT);
		$title = NEW_CASERVER_FOR.' '.$parent['title'];
		$caclient = array();
		$fields = array('title', 'country', 'state', 'locality',
				'organization', 'section', 'cn', 'email');
		foreach($fields as $f)
			if(isset($args[$f]))
				$caserver[$f] = stripslashes($args[$f]);
		include('./modules/pki/caserver_update.tpl');
	}


	//PkiModule::config_update
	protected function config_update($args)
	{
		global $error;
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);

		if(isset($error) && strlen($error))
			_error($error);
		return $this->admin(array());
	}


	//PkiModule::_default
	protected function _default($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(isset($args['id']))
			return $this->display($args);
		print('<h1 class="title pki">'._html_safe(
					PUBLIC_KEY_INFRASTRUCTURE)."</h1>\n");
		//$enabled = " AND enabled='1'";
		$enabled = '';
		$this->_display_ca_list_type(FALSE, 'ca', CA_LIST, $enabled);
	}


	//PkiModule::delete
	protected function delete($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		require_once('./system/content.php');
		if(isset($args['id'])) //FIXME actually implement
			_content_delete($args['id']);
	}


	//PkiModule::disable
	protected function disable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		require_once('./system/content.php');
		if(isset($args['id']))
			_content_disable($args['id']);
	}


	//PkiModule::display
	protected function display($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		if(!isset($args['id']))
			return _error(INVALID_ARGUMENT);
		if(_sql_single('SELECT ca_id FROM daportal_ca WHERE '
					."ca_id='".$args['id']."'")
				== $args['id'])
			return $this->_display_ca($args);
		$types = array('caclient', 'caserver');
		foreach($types as $t)
			if(_sql_single('SELECT '.$t.'_id FROM daportal_'.$t
						.' WHERE '
						.$t."_id='".$args['id']."'")
					== $args['id'])
				return $this->_display_type($args, $t);
		return _error(INVALID_ARGUMENT);
	}

private function _display_ca($args)
{
	//$enabled = " AND enabled='1'";
	$enabled = '';
	$ca = _sql_array('SELECT ca_id AS id, title, country, state, locality'
			.', organization, section, cn, email, parent'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			.$enabled." AND ca_id='".$args['id']."'");
	if(!is_array($ca) || count($ca) != 1)
		return _error(INVALID_ARGUMENT);
	$ca = $ca[0];
	include('./modules/pki/ca_display.tpl');
	$this->_display_ca_single($ca['parent'], PARENT_CA);
	$this->_display_ca_list_type($ca['id'], 'ca', CA_LIST, $enabled);
	$this->_display_ca_list_type($ca['id'], 'caserver', CASERVER_LIST, $enabled);
	$this->_display_ca_list_type($ca['id'], 'caclient', CACLIENT_LIST, $enabled);
}

private function _display_ca_list_type($id, $type, $title, $enabled)
{
	print('<h2 class="title '.$type.'">'._html_safe($title)."</h2>\n");
	$parent = $id ? " AND parent='".$id."'" : '';
	$res = _sql_array('SELECT '.$type.'_id AS id, title, country, state'
			.', locality, organization, section, cn, email, enabled'
			.' FROM daportal_'.$type.', daportal_content'
			.' WHERE daportal_'.$type.'.'.$type.'_id'
			.'=daportal_content.content_id'.$enabled.$parent
			.' ORDER BY title ASC');
	if(!is_array($res))
		return _error('Could not list certificates');
	$classes = array('country' => COUNTRY, 'state' => STATE,
			'locality' => LOCALITY, 'organization' => ORGANIZATION,
			'section' => SECTION, 'cn' => CN, 'email' => EMAIL);
	if($enabled == '')
		$classes = array_merge(array('enabled' => ENABLED), $classes);
	$keys = array_keys($classes);
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/'.$type.'.png';
		$res[$i]['thumbnail'] = 'icons/48x48/'.$type.'.png';
		$res[$i]['module'] = 'pki';
		$res[$i]['action'] = 'display';
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		$res[$i]['tag'] = $res[$i]['title'];
		foreach($keys as $k)
			$res[$i][$k] = _html_safe($res[$i][$k]);
		if($enabled == '')
		{
			$res[$i]['apply_module'] = 'pki';
			$res[$i]['apply_id'] = $res[$i]['id'];
			$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE
				? 'enabled' : 'disabled';
			$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		}
		$res[$i]['email'] = '<a href="mailto:'.$res[$i]['email'].'">'
			.$res[$i]['email'].'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('class' => 'new', 'link' => _module_link('pki',
				$type.'_insert', FALSE, FALSE, isset($id)
				? 'parent='.$id : ''));
	if($enabled == '')
	{
		$toolbar[] = array();
		$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
				'action' => 'disable');
		$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
				'action' => 'enable');
		$toolbar[] = array();
		$toolbar[] = array('title' => RENEW, 'class' => 'new', //XXX
				'action' => 'renew', 'confirm' => 'renew');
		$toolbar[] = array('title' => REVOKE, 'class' => 'delete', //XXX
				'action' => 'revoke', 'confirm' => 'revoke');
		$toolbar[] = array();
		$toolbar[] = array('title' => DELETE, 'class' => 'delete',
				'action' => 'delete', 'confirm' => 'delete');
	}
	$args = array('entries' => $res, 'class' => $classes,
			'toolbar' => $toolbar, 'view' => 'details');
	if($enabled == '')
	{
		$args['module'] = 'pki';
		$args['action'] = 'admin';
	}
	_module('explorer', 'browse_trusted', $args);
}

private function _display_ca_single($id, $title)
{
	if(!is_numeric($id))
		return;
	print('<h2 class="title ca">'._html_safe($title)."</h2>\n");
	$res = _sql_array('SELECT ca_id AS id, title, country, state'
			.', locality, organization, section, cn, email'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			." AND enabled='1' AND ca_id='$id'");
	$classes = array('country' => COUNTRY, 'state' => STATE,
			'locality' => LOCALITY, 'organization' => ORGANIZATION,
			'section' => SECTION, 'cn' => CN, 'email' => EMAIL);
	if(!is_array($res) || count($res) != 1)
		$res = array();
	else
	{
		$keys = array_keys($classes);
		$res[0]['icon'] = 'icons/16x16/ca.png';
		$res[0]['thumbnail'] = 'icons/48x48/ca.png';
		$res[0]['module'] = 'pki';
		$res[0]['action'] = 'display';
		$res[0]['name'] = _html_safe($res[0]['title']);
		$res[0]['tag'] = $res[0]['title'];
		foreach($keys as $k)
			$res[0][$k] = _html_safe($res[0][$k]);
		$res[0]['email'] = '<a href="mailto:'.$res[0]['email'].'">'
			.$res[0]['email'].'</a>';
	}
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => $classes, 'view' => 'details'));
}

private function _display_type($args, $type)
{
	global $user_id;

	$enabled = " AND enabled='1'";
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$enabled = '';
	$res = _sql_array('SELECT '.$type.'_id AS id, title, country'
			.', state, locality, organization, section, cn, email'
			.', parent FROM daportal_'.$type.', daportal_content'
			." WHERE daportal_$type.$type"."_id"
			.'=daportal_content.content_id'
			.$enabled." AND $type"."_id='".$args['id']."'");
	if(!is_array($res) || count($res) != 1)
		return _error(INVALID_ARGUMENT);
	$$type = $res[0];
	include('./modules/pki/'.$type.'_display.tpl');
	$this->_display_ca_single($res[0]['parent'], PARENT_CA);
}


	//PkiModule::enable
	protected function enable($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return _error(PERMISSION_DENIED);
		require_once('./system/content.php');
		if(isset($args['id']))
			_content_enable($args['id']);
	}


//PkiModule::renew
protected function renew($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	//FIXME only allow POST
	$type = 'caserver'; //XXX fix this
	if(!isset($args['id']) || !is_numeric($args['id']))
		return INVALID_ARGUMENT;

	//obtain certificate
	//FIXME this is code duplication all over the place
	$caclient = _sql_array('SELECT '.$type.'_id AS id, ccl.title AS title'
			.', daportal_ca.ca_id AS ca_id, cca.title AS ca'
			.' FROM daportal_'.$type.', daportal_content ccl'
			.', daportal_ca, daportal_content cca'
			.' WHERE daportal_'.$type.'.'.$type.'_id=ccl.content_id'
			.' AND daportal_'.$type.'.parent=daportal_ca.ca_id'
			.' AND daportal_ca.ca_id=cca.content_id'
			.' AND '.$type."_id='".$args['id']."'");
	if(!is_array($caclient) || count($caclient) != 1)
		return INVALID_ARGUMENT;
	$caclient = $caclient[0];

	$this->revoke($args); //XXX check if it failed or is already revoked

	//create signed certificate
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';
	$cadir = $root.'/'.$caclient['ca'];
	$ext = ($type == 'caclient') ? 'usr_cert' : 'srv_cert';
	$ecadir = escapeshellarg($cadir);
	$out = $cadir.'/certs/'.$caclient['title'].'.crt';
	$eout = escapeshellarg($out);
	$csr = $cadir.'/newreqs/'.$caclient['title'].'.csr';
	$ecsr = escapeshellarg($csr);
	if($this->_exec('openssl ca -config '.$ecadir.'/openssl.cnf'
				.' -extensions '.$ext.' -policy policy_anything'
				.' -out '.$eout.' -batch -infiles '.$ecsr,
				$output) != 0)
		return 'Could not renew certificate';
}


//PkiModule::revoke
protected function revoke($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	//FIXME only allow POST
	$type = 'caserver'; //XXX fix this
	if(!isset($args['id']) || !is_numeric($args['id']))
		return INVALID_ARGUMENT;

	//obtain certificate
	$caclient = _sql_array('SELECT '.$type.'_id AS id, ccl.title AS title'
			.', daportal_ca.ca_id AS ca_id, cca.title AS ca'
			.' FROM daportal_'.$type.', daportal_content ccl'
			.', daportal_ca, daportal_content cca'
			.' WHERE daportal_'.$type.'.'.$type.'_id=ccl.content_id'
			.' AND daportal_'.$type.'.parent=daportal_ca.ca_id'
			.' AND daportal_ca.ca_id=cca.content_id'
			.' AND '.$type."_id='".$args['id']."'");
	if(!is_array($caclient) || count($caclient) != 1)
		return INVALID_ARGUMENT;
	$caclient = $caclient[0];

	//revoke certificate
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';
	$cadir = $root.'/'.$caclient['ca'];
	$ecadir = escapeshellarg($cadir);
	$out = $cadir.'/certs/'.$caclient['title'].'.crt';
	$eout = escapeshellarg($out);
	if($this->_exec('openssl ca -config '.$ecadir.'/openssl.cnf'
				.' -revoke '.$eout, $output) != 0)
		return 'Could not revoke certificate';
}


//PkiModule::system
protected function system($args)
{
	global $title, $error, $user_id;

	$title.=' - '.PKI;
	if(!isset($args['action']))
		return;
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return;
	if($_SERVER['REQUEST_METHOD'] == 'GET')
		switch($args['action'])
		{
			case 'ca_export':
				$error = $this->_system_ca_export($args);
				break;
			case 'ca_import':
				$error = $this->_system_ca_export($args,
						'inline');
				break;
		}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
		switch($args['action'])
		{
			case 'ca_insert':
				$error = $this->_system_ca_insert($args);
				break;
			case 'caclient_export':
				$error = $this->_system_export($args,
						'caclient');
				break;
			case 'caclient_insert':
				$error = $this->_system_insert($args,
						'caclient');
				break;
			case 'caclient_renew':
				$error = $this->_system_renew($args,
						'caclient');
				break;
			case 'caserver_export':
				$error = $this->_system_export($args,
						'caserver');
				break;
			case 'caserver_insert':
				$error = $this->_system_insert($args,
						'caserver');
				break;
			case 'caserver_renew':
				$error = $this->_system_renew($args,
						'caserver');
				break;
			case 'config_update':
				$error = $this->_system_config_update($args);
				break;
		}
	}

private function _system_ca_export($args, $disposition = 'attachment')
{
	global $user_id;

	$enabled = " AND enabled='1'";
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$enabled = '';
	if(!isset($args['id']) || !is_numeric($args['id']))
		return INVALID_ARGUMENT;
	$sql = 'SELECT title FROM daportal_ca, daportal_content'
		.' WHERE daportal_ca.ca_id=daportal_content.content_id'
		.$enabled." AND ca_id='".$args['id']."'";
	if(($name = _sql_single($sql)) == FALSE)
		return INVALID_ARGUMENT;
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';
	$cadir = $root.'/'.$name;
	$ecadir = escapeshellarg($cadir);
	$output = array();
	if($this->_exec('openssl x509 -in '.$ecadir.'/cacert.crt', $output) != 0)
		return 'Could not export certificate';
	$crt = implode("\n", $output);
	header('Content-Type: application/x-x509-ca-cert');
	header('Content-Length: '.strlen($crt));
	header('Content-Disposition: '.$disposition.'; filename=cacert.crt');
	print($crt);
	exit(0);
}

private function _system_ca_insert($args)
{
	global $user_id;

	//check permissions
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;

	//validate title
	if(!isset($args['title'])
			|| strchr($args['title'], '/') != FALSE
			|| $args['title'] == '..')
		return INVALID_ARGUMENT;

	//fetch configuration
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';

	//validate parent
	if(isset($args['parent']) && is_numeric($args['parent']))
	{
		if(($parent = $this->_ca_get($args['parent'])) == FALSE)
			return INVALID_ARGUMENT;
		$pcadir = $root.'/'.$parent['title'];
		if(!is_dir($pcadir))
			return 'Parent infrastructure not found';
	}

	//validate unicity
	$cadir = $root.'/'.stripslashes($args['title']);
	if(is_dir($cadir))
		return 'A CA by that name already exists';

	//create directories
	$dirs = array('certs', 'crl', 'newcerts', 'newreqs', 'private');
	foreach($dirs as $d)
		if($this->_mkdir($cadir.'/'.$d, 0700, TRUE) != TRUE)
		{
			$this->_insert_cleanup($cadir, $dirs);
			return 'Could not create directories';
		}

	//validate rest of input
	$fields = array('title', 'country', 'state', 'locality', 'organization',
			'section', 'cn', 'email');
	$ca = array();
	foreach($fields as $f)
	{
		if(!isset($args[$f]))
			$args[$f] = '';
		/* XXX will no longer be necessary */
		$ca[$f] = stripslashes($args[$f]);
	}

	//create files
	$files = array('index.txt', 'openssl.cnf', 'serial');
	if(touch($cadir.'/index.txt') !== TRUE
			|| $this->_ssl_config($root, $cadir, $ca) === FALSE
			|| ($fp = fopen($cadir.'/serial', 'w')) === FALSE
			|| fwrite($fp, "01\n") === FALSE
			|| fclose($fp) === FALSE)
	{
		$this->_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create files';
	}

	//create CA certificate or certificate request
	$files[] = 'private/cacert.key';
	$files[] = isset($parent) ? 'cacert.csr' : 'cacert.crt';
	$ecadir = escapeshellarg($cadir);
	$subject = $this->_subject_from_ca($ca);
	$output = array();
	$sslargs = isset($parent) ? ' -out '.$ecadir.'/cacert.csr'
		: ' -x509 -out '.$ecadir.'/cacert.crt';
	if($this->_exec('openssl req -config '.$ecadir.'/openssl.cnf -nodes'
				.' -new'.$sslargs.' -days 3650'
				.' -keyout '.$ecadir.'/private/cacert.key'
				.' -subj '.escapeshellarg($subject), $output)
			!= 0)
	{
		$this->_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create certificate';
	}

	//sign the certificate if necessary
	if(isset($parent))
	{
		$files[] = 'cacert.crt';
		$epcadir = escapeshellarg($pcadir);
		if($this->_exec('openssl ca -config '.$epcadir.'/openssl.cnf'
					.' -extensions v3_ca'
					.' -policy policy_anything'
					.' -out '.$ecadir.'/cacert.crt -batch'
					.' -infiles '.$ecadir.'/cacert.csr',
					$output) != 0)
		{
			$this->_insert_cleanup($cadir, $dirs, $files);
			return 'Could not sign certificate';
		}
	}

	//insert in database
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], '', 1)) === FALSE)
	{
		$this->_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert content';
	}
	$sql = 'INSERT INTO daportal_ca (ca_id';
	if(isset($parent))
		$sql.=', parent';
	$sql.=', country, state, locality, organization, section, cn, email)'
		." VALUES ('$id'";
	if(isset($parent))
		$sql.=", '".$parent['id']."'";
	$sql.=", '".$args['country']."'"
		.", '".$args['state']."', '".$args['locality']."'"
		.", '".$args['organization']."', '".$args['section']."'"
		.", '".$args['cn']."', '".$args['email']."')";
	if(_sql_query($sql) === FALSE)
	{
		_content_delete($id);
		$this->_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert CA';
	}

	//display the CA
	header('Location: '._module_link('pki', 'display', $id));
	exit(0);
}

private function _system_export($args, $type, $disposition = 'attachment')
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	if(!isset($args['id']) || !is_numeric($args['id'])
			|| !isset($args['key']))
		return INVALID_ARGUMENT;
	$caclient = _sql_array('SELECT '.$type.'_id AS id, ccl.title AS title'
			.', daportal_ca.ca_id AS ca_id, cca.title AS ca'
			.' FROM daportal_'.$type.', daportal_content ccl'
			.', daportal_ca, daportal_content cca'
			.' WHERE daportal_'.$type.'.'.$type.'_id=ccl.content_id'
			.' AND daportal_'.$type.'.parent=daportal_ca.ca_id'
			.' AND daportal_ca.ca_id=cca.content_id'
			.' AND '.$type."_id='".$args['id']."'");
	if(!is_array($caclient) || count($caclient) != 1)
		return INVALID_ARGUMENT;
	$caclient = $caclient[0];
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';
	$cadir = $root.'/'.$caclient['ca'];
	$crt = $cadir.'/newcerts/'.$caclient['title'].'.crt';
	$ecrt = escapeshellarg($crt);
	$out = $cadir.'/certs/'.$caclient['title'].'.crt';
	$eout = escapeshellarg($out);
	$ekey = escapeshellarg(stripslashes($args['key']));
	//FIXME detect errors with popen(), secure password
	if($type == 'caserver')
	{
		$cmd = 'openssl x509 -in '.$eout;
		if(($fp2 = popen($cmd, 'r')) == FALSE)
			return 'Could not export certificate';
		$cmd = 'openssl rsa -in '.$ecrt.' -des3'
			.' -passout pass:'.$ekey;
		if(($fp = popen($cmd, 'r')) == FALSE)
			return 'Could not export certificate';
		header('Content-Type: application/x-x509-ca-cert');
		header('Content-Disposition: '.$disposition
				.'; filename=cert.crt');
		fpassthru($fp2);
		fclose($fp2);
	}
	else
	{
		$ecadir = escapeshellarg($cadir);
		$cmd = 'openssl pkcs12 -export -in '.$eout
			.' -certfile '.$ecadir.'/cacert.crt'
			.' -passout pass:'.$ekey;
		if(isset($args['private'])) //FIXME check this works
			$cmd.=' -inkey '.$ecrt;
		else
			$cmd.=' -nokeys';
		if(($fp = popen($cmd, 'r')) == FALSE)
			return 'Could not export certificate';
		header('Content-Type: application/x-pkcs12');
		header('Content-Disposition: '.$disposition
				.'; filename=cert.p12');
	}
	fpassthru($fp);
	fclose($fp);
	exit(0);
}

private function _system_insert($args, $type)
{
	global $user_id;

	//check permissions
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;

	//validate title and parent
	if(!isset($args['title'])
			|| strchr($args['title'], '/') != FALSE
			|| $args['title'] == '..'
			|| !isset($args['parent'])
			|| !is_numeric($args['parent']))
		return INVALID_ARGUMENT;
	$caclient = array('title' => stripslashes($args['title']));

	//fetch configuration
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';

	//get parent
	if(($parent = $this->_ca_get($args['parent'])) == FALSE)
		return INVALID_ARGUMENT;
	$cadir = $root.'/'.$parent['title'];
	if(!is_dir($cadir))
		return 'Parent infrastructure not found';

	//validate unicity
	$csr = '/newreqs/'.$caclient['title'].'.csr';
	$crt = '/newcerts/'.$caclient['title'].'.crt';
	$out = '/certs/'.$caclient['title'].'.crt';
	$files = array($csr, $crt, $out);
	$csr = $cadir.$csr;
	$crt = $cadir.$crt;
	$out = $cadir.$out;
	if(file_exists($csr) || file_exists($crt) || file_exists($out))
		return 'A client by that name already exists';

	//validate rest of input
	$fields = array('country', 'state', 'locality', 'organization',
			'section', 'cn', 'email');
	foreach($fields as $f)
	{
		if(!isset($args[$f]))
			$args[$f] = '';
		$caclient[$f] = stripslashes($args[$f]);
	}

	//create certificate request
	$ecadir = escapeshellarg($cadir);
	$ecrt = escapeshellarg($crt);
	$output = array();
	$ext = ($type == 'caclient') ? 'usr_cert' : 'srv_cert';
	if($this->_exec('openssl req -config '.$ecadir.'/openssl.cnf'
				.' -nodes -new -x509'
				.' -extensions '.$ext.' -days 365'
				.' -keyout '.$ecrt.' -out '.$ecrt
				.' -subj '.escapeshellarg(
					$this->_subject_from_ca($caclient)),
				$output) != 0)
	{
		$this->_insert_cleanup($cadir, FALSE, $files);
		return 'Could not generate certificate';
	}

	//create signing request
	$ecsr = escapeshellarg($csr);
	if($this->_exec('openssl x509 -x509toreq -in '.$ecrt.' -out '.$ecsr
				.' -signkey '.$ecrt, $output) != 0)
	{
		$this->_insert_cleanup($cadir, FALSE, $files);
		return 'Could not generate signing request';
	}

	//create signed certificate
	$eout = escapeshellarg($out);
	if($this->_exec('openssl ca -config '.$ecadir.'/openssl.cnf'
				.' -extensions '.$ext.' -policy policy_anything'
				.' -out '.$eout.' -batch -infiles '.$ecsr,
				$output) != 0)
	{
		$this->_insert_cleanup($cadir, FALSE, $files);
		return 'Could not generate signed certificate';
	}

	//insert in database
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], '', 1)) === FALSE)
	{
		$this->_insert_cleanup($cadir, FALSE, $files);
		return 'Could not insert content';
	}
	if(_sql_query('INSERT INTO daportal_'.$type.' ('.$type.'_id, parent'
			.', country, state, locality, organization, section, cn'
			.', email)'." VALUES ('$id', '".$parent['id']."'"
			.", '".$args['country']."', '".$args['state']."'"
			.", '".$args['locality']."'"
			.", '".$args['organization']."', '".$args['section']."'"
			.", '".$args['cn']."', '".$args['email']."')")
			=== FALSE)
	{
		_content_delete($id);
		$this->_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert CA';
	}

	//display the client
	header('Location: '._module_link('pki', 'display', $id));
	exit(0);
}

	private function _system_config_update($args)
	{
		global $user_id;

		require_once('./system/user.php');
		if(!_user_admin($user_id))
			return PERMISSION_DENIED;
		_config_update('pki', $args);
		header('Location: '._module_link('pki', 'admin'));
		exit(0);
	}
}

?>
