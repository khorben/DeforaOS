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
$text['ADD_A_USER'] = 'Add a user';
$text['ADMINISTRATION'] = 'Administration';
$text['BROWSE_FILES'] = 'Browse files';
$text['CA_LIST'] = 'CA list';
$text['CACLIENT_LIST'] = 'Client list';
$text['CASERVER_LIST'] = 'Server list';
$text['CN'] = 'CN';
$text['COMMON_NAME'] = 'Common name';
$text['COUNTRY'] = 'Country';
$text['EMAIL'] = 'e-mail';
$text['EXPORT'] = 'Export';
$text['IMPORT'] = 'Import';
$text['LOCALITY'] = 'Locality';
$text['NEW_CA'] = 'New CA';
$text['ORGANIZATION'] = 'Organization';
$text['PAPADAM'] = 'Papadam';
$text['PAPADAM_ADMINISTRATION'] = 'Papadam administration';
$text['REVOKE_A_USER'] = 'Revoke a user';
$text['SECTION'] = 'Section';
$text['SELF_SIGNED'] = 'Self-signed';
$text['SETTINGS'] = 'Settings';
$text['STATE'] = 'State';
global $lang;
if($lang == 'fr')
{
	$text['ADD_A_USER'] = 'Ajouter un utilisateur';
	$text['BROWSE_FILES'] = 'Explorateur de fichiers';
	$text['REVOKE_A_USER'] = 'Révoquer un utilisateur';
}
_lang($text);


//private
//get a CA
function _ca_get($id)
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

//exec
//helper function for exec()
function _exec($cmd, &$output)
{
	$ret = 1;
	_info($cmd);
	exec($cmd, $output, $ret);
	return $ret;
}


//insert_cleanup
//cleanup a CA or client files when an error occurs
function _insert_cleanup($cadir, $dirs = FALSE, $files = FALSE)
{
	if(is_array($files))
		foreach($files as $f)
			@unlink($cadir.'/'.$f);
	if(is_array($dirs))
		foreach($dirs as $d)
			@rmdir($cadir.'/'.$d);
	@rmdir($cadir);
}


//mkdir
//variant of mkdir() with optional recursivity
function _mkdir($pathname, $mode = 0777, $recursive = FALSE)
{
	_info('mkdir '.$pathname);
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


//public
//papadam_admin
function papadam_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title papadam">'._html_safe(PAPADAM_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('pki')))
	{
		print('<h2 class="title settings"> '._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'papadam';
		$action = 'config_update';
		include('./system/config.tpl');
	}
	print('<h2 class="title papadam"> '._html_safe(CA_LIST)."</h2>\n");
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
		$res[$i]['module'] = 'pki';
		$res[$i]['apply_module'] = 'pki';
		$res[$i]['action'] = 'update';
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
			'link' => _module_link('papadam', 'ca_insert'));
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => $classes, 'module' => 'pki',
				'action' => 'admin', 'toolbar' => $toolbar,
				'view' => 'details'));
}


//ca_insert
function papadam_ca_insert($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	$title = NEW_CA;
	$ca = array();
	$fields = array('title', 'country', 'state', 'locality', 'organization',
			'section', 'cn', 'email');
	foreach($fields as $f)
		if(isset($args[$f]))
			$ca[$f] = stripslashes($args[$f]);
	$parent = _sql_array('SELECT ca_id AS id, title'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			." AND enabled='1'");
	if(isset($args['parent']) && _ca_get($args['parent']) != FALSE)
		$parent_id = $args['parent'];
	include('./modules/papadam/ca_update.tpl');
}


//config_update
function papadam_config_update($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	return papadam_admin(array());
}


//default
function papadam_default($args)
{
	if(isset($args['id']))
		return papadam_display($args);
	print('<h1 class="title papadam">'._html_safe(PAPADAM)."</h1>\n");
	$entries = array();
	$entries[] = array('name' => ADMINISTRATION,
			'thumbnail' => 'icons/48x48/admin.png',
			'module' => 'admin', 'action' => 'default');
	$entries[] = array('name' => ADD_A_USER,
			'thumbnail' => 'icons/48x48/users.png',
			'module' => 'pki', 'action' => 'caclient_insert');
	$entries[] = array('name' => BROWSE_FILES,
			'thumbnail' => 'icons/48x48/download.png',
			'module' => 'browser', 'action' => 'default');
	$entries[] = array('name' => REVOKE_A_USER,
			'thumbnail' => 'icons/48x48/user.png');
	_module('explorer', 'browse', array('entries' => $entries,
			'view' => 'thumbnails', 'toolbar' => 0));
}


//display
function papadam_display($args)
{
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	if(_sql_single('SELECT ca_id FROM daportal_ca WHERE '
				."ca_id='".$args['id']."'") == $args['id'])
		return _display_ca($args);
	$types = array('caclient', 'caserver');
	foreach($types as $t)
		if(_sql_single('SELECT '.$t.'_id FROM daportal_'.$t.' WHERE '
					.$t."_id='".$args['id']."'")
				== $args['id'])
			return _display_type($args, $t);
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
			.', organization, section, cn, email'
			.' FROM daportal_ca, daportal_content'
			.' WHERE daportal_ca.ca_id=daportal_content.content_id'
			.$enabled." AND ca_id='".$args['id']."'");
	if(!is_array($ca) || count($ca) != 1)
		return _error(INVALID_ARGUMENT);
	$ca = $ca[0];
	include('./modules/pki/ca_display.tpl');
	_display_ca_list_type($ca['id'], 'ca', CA_LIST, $enabled);
	_display_ca_list_type($ca['id'], 'caserver', CASERVER_LIST, $enabled);
	_display_ca_list_type($ca['id'], 'caclient', CACLIENT_LIST, $enabled);
}

function _display_ca_list_type($id, $type, $title, $enabled)
{
	print("<h2 class=\"title $type\">"._html_safe($title)."</h2>\n");
	$parent = $id ? " AND parent='".$id."'" : '';
	$res = _sql_array('SELECT '.$type.'_id AS id, title, country, state'
			.', locality, organization, section, cn, email'
			.' FROM daportal_'.$type.', daportal_content'
			.' WHERE daportal_'.$type.'.'.$type.'_id'
			.'=daportal_content.content_id'.$enabled.$parent);
	if(!is_array($res))
		return _error('Could not list certificates');
	$classes = array('country' => COUNTRY, 'state' => STATE,
			'locality' => LOCALITY, 'organization' => ORGANIZATION,
			'section' => SECTION, 'cn' => CN, 'email' => EMAIL);
	$keys = array_keys($classes);
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'pki';
		$res[$i]['action'] = 'display';
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		foreach($keys as $k)
			$res[$i][$k] = _html_safe($res[$i][$k]);
		$res[$i]['email'] = '<a href="mailto:'.$res[$i]['email'].'">'
			.$res[$i]['email'].'</a>';
	}
	$toolbar = array();
	$toolbar[] = array('class' => 'new', 'link' => _module_link('pki',
				$type.'_insert', FALSE, FALSE, isset($id)
				? 'parent='.$id : ''));
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => $classes, 'toolbar' => $toolbar,
				'view' => 'details'));
}

function _display_type($args, $type)
{
	global $user_id;

	$enabled = " AND enabled='1'";
	require_once('./system/user.php');
	if(_user_admin($user_id))
		$enabled = '';
	$caclient = _sql_array('SELECT '.$type.'_id AS id, title, country'
			.', state, locality, organization, section, cn, email'
			.' FROM daportal_'.$type.', daportal_content'
			." WHERE daportal_$type.$type"."_id"
			.'=daportal_content.content_id'
			.$enabled." AND $type"."_id='".$args['id']."'");
	if(!is_array($caclient) || count($caclient) != 1)
		return _error(INVALID_ARGUMENT);
	$$type = $caclient[0];
	include('./modules/pki/'.$type.'_display.tpl');
}


//system
function papadam_system($args)
{
	global $title, $error;

	$title.=' - '.PAPADAM;
	if(!isset($args['action']))
		return;
	if($_SERVER['REQUEST_METHOD'] == 'POST')
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

function _system_ca_insert($args)
{
	global $user_id;

	//check permissions
	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;

	//validate title
	if(!isset($args['title'])
			|| strlen($args['title']) == 0
			|| strchr($args['title'], '/') != FALSE
			|| $args['title'] == '..')
		return INVALID_ARGUMENT;

	//fetch configuration
	if(($root = _config_get('pki', 'root')) == FALSE)
		return 'Could not fetch the root directory';

	//validate parent
	if(!isset($args['parent']) || !is_numeric($args['parent']))
		return INVALID_ARGUMENT;
	if(($parent = _ca_get($args['parent'])) == FALSE)
		return INVALID_ARGUMENT;
	$pcadir = $root.'/'.$parent['title'];
	if(!is_dir($pcadir))
		return 'Parent infrastructure not found';

	//validate unicity
	$cadir = $root.'/'.stripslashes($args['title']);
	if(is_dir($cadir))
		return 'A CA by that name already exists';

	//create directories
	$dirs = array('certs', 'crl', 'newcerts', 'newreqs', 'private');
	foreach($dirs as $d)
		if(_mkdir($cadir.'/'.$d, 0700, TRUE) != TRUE)
		{
			_insert_cleanup($cadir, $dirs);
			return 'Could not create directories';
		}

	//validate rest of input
	$fields = array('title', 'country', 'state', 'locality', 'organization',
			'section', 'cn', 'email');
	foreach($fields as $f)
	{
		if(!isset($args[$f]))
			$args[$f] = '';
		$ca[$f] = stripslashes($args[$f]);
	}

	//create files
	$files = array('index.txt', 'openssl.cnf', 'serial');
	if(touch($cadir.'/index.txt') != TRUE
			|| _ssl_config($root, $cadir, $ca) == FALSE
			|| ($fp = fopen($cadir.'/serial', 'w')) == FALSE
			|| fwrite($fp, "01\n") == FALSE
			|| fclose($fp) == FALSE)
	{
		_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create files';
	}

	//create CA certificate or certificate request
	$files[] = 'private/cacert.key';
	$files[] = 'cacert.csr';
	$ecadir = escapeshellarg($cadir);
	$output = array();
	$sslargs = ' -out '.$ecadir.'/cacert.csr';
	$cmd = 'cd '.escapeshellarg($root).' && register';
	/* FIXME escapeshellarg() does not return "''" for an empty value
	         as a consequence empty values are erroneously skipped */
	$cmd .= ' -c '.escapeshellarg($ca['country']);
	$cmd .= ' -e '.escapeshellarg($ca['email']);
	$cmd .= ' -l '.escapeshellarg($ca['locality']);
	$cmd .= ' -n '.escapeshellarg($ca['cn']);
	$cmd .= ' -o '.escapeshellarg($ca['organization']);
	$cmd .= ' -s '.escapeshellarg($ca['state']);
	$cmd .= ' -u '.escapeshellarg($ca['section']);
	$cmd .= ' -t '.escapeshellarg($ca['title']);
	if(_exec($cmd, $output) != 0)
	{
		_insert_cleanup($cadir, $dirs, $files);
		return 'Could not create certificate';
	}

	//insert in database
	require_once('./system/content.php');
	if(($id = _content_insert($args['title'], '', 1)) == FALSE)
	{
		_insert_cleanup($cadir, $dirs, $files);
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
	if(_sql_query($sql) == FALSE)
	{
		_content_delete($id);
		_insert_cleanup($cadir, $dirs, $files);
		return 'Could not insert CA';
	}

	//display the CA
	header('Location: '._module_link('papadam', 'display', $id));
	exit(0);
}

function _system_config_update($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return PERMISSION_DENIED;
	_config_update('pki', $args);
	header('Location: '._module_link('papadam', 'admin'));
	exit(0);
}

?>
