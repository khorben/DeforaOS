<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <pierre.pronchery@duekin.com>
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



//check url
if(!ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['CA_LIST'] = 'CA list';
$text['CN'] = 'CN';
$text['COUNTRY'] = 'Country';
$text['EMAIL'] = 'e-mail';
$text['LOCALITY'] = 'Locality';
$text['NEW_CA'] = 'New CA';
$text['ORGANIZATION'] = 'Organization';
$text['PKI'] = 'PKI';
$text['PKI_ADMINISTRATION'] = 'PKI administration';
$text['PUBLIC_KEY_INFRASTRUCTURE'] = 'Public key infrastructure';
$text['SECTION'] = 'Section';
$text['SETTINGS'] = 'Settings';
$text['STATE'] = 'State';
$text['UNIT'] = 'Unit';
global $lang;
_lang($text);


//private
//exec
//helper function for exec()
function _exec($cmd, &$output)
{
	$ret = 1;
	_info($cmd);
	exec($cmd, $output, $ret);
	return $ret;
}


//mkdir
//variant of mkdir() with optional recursivity
function _mkdir($pathname, $mode = 0777, $recursive = FALSE)
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


//ssl_config
//creates an openssl.cnf file for a new CA
function _ssl_config($home, $cadir, $ca)
{
	if(($fp = fopen($cadir.'/openssl.cnf', 'w')) == FALSE)
		return FALSE;
	ob_start('_config_true');
	include('./modules/pki/openssl.cnf.tpl');
	fwrite($fp, ob_get_contents());
	ob_end_clean();
	if(fclose($fp) != TRUE)
		return FALSE;
	return TRUE;
}

function _config_true($void)
{
	return TRUE;
}


//subject_from_ca
//returns the subject line for a given CA specification
function _subject_from_ca($ca)
{
	//FIXME should escape slashes
	return '/C='.escapeshellarg($ca['country'])
		.'/ST='.escapeshellarg($ca['state'])
		.'/L='.escapeshellarg($ca['locality'])
		.'/O='.escapeshellarg($ca['organization'])
		.'/OU='.escapeshellarg($ca['unit'])
		.'/CN='.escapeshellarg($ca['title'])
		.'/emailAddress='.escapeshellarg($ca['email']);
}


//public
//pki_admin
function pki_admin($args)
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
	print('<h2 class="title pki"> '._html_safe(CA_LIST)."</h2>\n");
	$sql = 'SELECT ca_id AS id, title, enabled, country, state, locality'
		.', organization, unit, section, cn, email'
		.' FROM daportal_ca, daportal_content'
		.' WHERE daportal_ca.ca_id=daportal_content.content_id';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list CAs');
	$cols = array('country', 'state', 'locality', 'organization', 'unit',
			'section', 'cn', 'email');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'pki';
		$res[$i]['action'] = 'update';
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
			.$res[$i]['enabled'].'.png" alt="'
			.$res[$i]['enabled'].'" title="'
			.($res[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
		foreach($cols as $c)
			$res[$i][$c] = _html_safe($res[$i][$c]);
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CA, 'class' => 'new',
			'link' => _module_link('pki', 'ca_new'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'country' => COUNTRY, 'state' => STATE,
					'locality' => LOCALITY,
					'organization' => ORGANIZATION,
					'unit' => UNIT, 'section' => SECTION,
					'cn' => CN, 'email' => EMAIL),
				'toolbar' => $toolbar, 'view' => 'details'));
}


//ca_export
function pki_ca_export($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return pki_ca_display($args);
}


//ca_import
function pki_ca_import($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return pki_ca_display($args);
}


//ca_insert
function pki_ca_insert($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	$title = NEW_CA;
	$ca = array();
	$fields = array('title', 'country', 'state', 'locality', 'organization',
			'unit', 'section', 'cn', 'email');
	foreach($fields as $f)
		if(isset($args[$f]))
			$ca[$f] = stripslashes($args[$f]);
	include('./modules/pki/ca_update.tpl');
}


//ca_new
function pki_ca_new($args)
{
	$title = NEW_CA;
	$parent = _sql_array('SELECT ca_id AS id, title'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			." AND enabled='1'");
	include('./modules/pki/ca_update.tpl');
}


//config_update
function pki_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	//FIXME what to do here?
}


//default
function pki_default($args)
{
	//FIXME implement user_id

	if(isset($args['id']))
		return pki_display($args);
	print('<h1 class="title pki">'._html_safe(PUBLIC_KEY_INFRASTRUCTURE)
			."</h1>\n");
	print('<h2 class="title pki"> '._html_safe(CA_LIST)."</h2>\n");
	$sql = 'SELECT ca_id AS id, title, enabled, country, state, locality'
		.', organization, unit, section, cn, email'
		.' FROM daportal_ca, daportal_content'
		.' WHERE daportal_ca.ca_id=daportal_content.content_id'
		." AND enabled='1'";
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list CAs');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'pki';
		$res[$i]['action'] = 'display';
		$res[$i]['name'] = $res[$i]['title'];
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
			.$res[$i]['enabled'].'.png" alt="'
			.$res[$i]['enabled'].'" title="'
			.($res[$i]['enabled'] == 'enabled'
					? ENABLED : DISABLED).'"/>';
	}
	_module('explorer', 'browse', array('entries' => $res,
				'class' => array('country' => COUNTRY,
					'state' => STATE,
					'locality' => LOCALITY,
					'organization' => ORGANIZATION,
					'unit' => UNIT, 'section' => SECTION,
					'cn' => CN, 'email' => EMAIL),
				'view' => 'details'));
}


//display
function pki_display($args)
{
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	if(_sql_single('SELECT ca_id FROM daportal_ca WHERE '
				."ca_id='".$args['id']."'") == $args['id'])
		return _display_ca($args);
	return _error(INVALID_ARGUMENT);
}

function _display_ca($args)
{
	global $user_id;

	$enabled = " AND enabled='1'";
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$enabled = '';
	$ca = _sql_array('SELECT ca_id AS id, title, country, state, locality'
			.', organization, unit, section, cn, email'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			.$enabled." AND ca_id='".$args['id']."'");
	if(!is_array($ca) || count($ca) != 1)
		return _error(INVALID_ARGUMENT);
	$ca = $ca[0];
	include('./modules/pki/ca_display.tpl');
}


//system
function pki_system($args)
{
	global $title, $error;

	$title.=' - '.PKI;
	if(!isset($args['action']))
		return;
	if($_SERVER['REQUEST_METHOD'] == 'GET')
		switch($args['action'])
		{
			case 'ca_export':
				$error = _system_ca_export($args);
				break;
			case 'ca_import':
				$error = _system_ca_export($args, 'inline');
				break;
		}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
		switch($args['action'])
		{
			case 'ca_insert':
				$error = _system_ca_insert($args);
				break;
			case 'config_update':
				$error = _system_config_update($args);
				break;
		}
}

function _system_ca_export($args, $disposition = 'attachment')
{
	global $user_id, $html;

	$enabled = " AND enabled='1'";
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$enabled = '';
	if(!isset($args['id']))
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
	if(_exec('openssl x509 -in '.$ecadir.'/cacert.crt', $output) != 0)
		return 'Could not export certificate';
	$crt = implode("\n", $output);
	$html = 0;
	header('Content-Type: application/x-x509-ca-cert');
	header('Content-Length: '.strlen($crt));
	header('Content-Disposition: '.$disposition.'; filename=cacert.crt');
	print($crt);
	exit(0);
}

function _system_ca_insert($args)
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
		$sql = 'SELECT ca_id, title FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			." AND enabled='1' AND ca_id='".$args['parent']."'";
		if(!is_array(($res = _sql_array($sql))) || count($res) != 1)
			return INVALID_ARGUMENT;
		$parent = $res[0];
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
		if(_mkdir($cadir.'/'.$d, 0700, TRUE) != TRUE)
		{
			_ca_insert_cleanup($cadir, $dirs);
			return 'Could not create directories';
		}

	//validate rest of input
	$cols = array('title', 'country', 'state', 'locality', 'organization',
			'unit', 'section', 'cn', 'email');
	foreach($cols as $c)
	{
		if(!isset($args[$c]))
			$args[$c] = '';
		$ca[$c] = stripslashes($args[$c]);
	}

	//create files
	$files = array('index.txt', 'openssl.cnf', 'serial');
	if(touch($cadir.'/index.txt') != TRUE
			|| _ssl_config($root, $cadir, $ca) == FALSE
			|| ($fp = fopen($cadir.'/serial', 'w')) == FALSE
			|| fwrite($fp, "01\n") == FALSE
			|| fclose($fp) == FALSE)
	{
		_ca_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create files';
	}

	//create CA certificate or certificate request
	$files[] = 'private/cacert.key';
	$files[] = isset($parent) ? 'cacert.csr' : 'cacert.crt';
	$ecadir = escapeshellarg($cadir);
	$subject = _subject_from_ca($ca);
	$output = array();
	$sslargs = isset($parent) ? ' -out '.$ecadir.'/cacert.csr'
		: ' -x509 -out '.$ecadir.'/cacert.crt';
	if(_exec('openssl req -config '.$ecadir.'/openssl.cnf -nodes -new'
				.$sslargs.' -days 3650'
				.' -keyout '.$ecadir.'/private/cacert.key'
				.' -subj '.$subject, $output) != 0)
	{
		_ca_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create certificate';
	}

	//sign the certificate if necessary
	if(isset($parent))
	{
		$files[] = 'cacert.crt';
		$epcadir = escapeshellarg($pcadir);
		if(_exec('openssl ca -config '.$epcadir.'/openssl.cnf'
					.' -extensions v3_ca'
					.' -policy policy_anything'
					.' -out '.$ecadir.'/cacert.crt -batch'
					.' -infiles '.$ecadir.'/cacert.csr',
					$output) != 0)
		{
			_ca_insert_cleanup($cadir, $dirs, $files);
			return 'Could not sign certificate';
		}
	}

	//insert in database
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], '', 1)) == FALSE)
	{
		_ca_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert content';
	}
	if(_sql_query('INSERT INTO daportal_ca (ca_id, country'
			.', state, locality, organization, unit, section, cn'
			.', email)'." VALUES ('$id', '".$args['country']."'"
			.", '".$args['state']."', '".$args['locality']."'"
			.", '".$args['organization']."', '".$args['unit']."'"
			.", '".$args['section']."', '".$args['cn']."'"
			.", '".$args['email']."')") == FALSE)
	{
		_content_delete($id);
		_ca_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert CA';
	}

	//display the CA
	header('Location: '._module_link('pki', 'display', $id));
	exit(0);
}

function _ca_insert_cleanup($cadir, $dirs = FALSE, $files = FALSE)
{
	if(is_array($files))
		foreach($files as $f)
			@unlink($cadir.'/'.$f);
	if(is_array($dirs))
		foreach($dirs as $d)
			@rmdir($cadir.'/'.$d);
	@rmdir($cadir);
}


function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^pki_([a-zA-Z_]+)$', $k, $regs))
			_config_set('pki', $regs[1], $args[$k], 0);
	header('Location: '._module_link('pki', 'admin'));
	exit(0);
}

?>
