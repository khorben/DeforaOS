<?php //$Id$
//Copyright (c) 2007 Pierre Pronchery <khorben@defora.org>
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
	exit(header('Location: ../../index.php'));


//lang
$text = array();
$text['DRAFTS'] = 'Drafts';
$text['INBOX'] = 'Inbox';
$text['MESSAGE_LIST'] = 'Message list';
$text['SENT'] = 'Sent';
$text['SETTINGS'] = 'Settings';
$text['SUBJECT'] = 'Subject';
$text['TRASH'] = 'Trash';
$text['WEBMAIL_ADMINISTRATION'] = 'Webmail administration';
$text['WRONG_PASSWORD'] = 'Wrong password';
global $lang;
if($lang == 'fr')
{
	$text['DRAFTS'] = 'Brouillons';
	$text['SUBJECT'] = 'Sujet';
	$text['TRASH'] = 'Corbeille';
	$text['WRONG_PASSWORD'] = 'Mot de passe incorrect';
}
_lang($text);



//private
function _webmail_connect($folder = 'INBOX')
{
	require_once('./system/config.php');
	if(!($server = _config_get('webmail', 'server')))
		$server = 'localhost';
	if(!isset($_SESSION['webmail']['username'])
			|| !isset($_SESSION['webmail']['password']))
		return FALSE;
	$mbox = imap_open('{'.$server.':993/imap/ssl/novalidate-cert}'.$folder,
			$_SESSION['webmail']['username'],
			$_SESSION['webmail']['password'],
			strlen($folder) ? 0 : OP_HALFOPEN);
	if(!$mbox)
	{
		unset($_SESSION['webmail']);
		_error('IMAP: '.imap_last_error(), 0);
		return FALSE;
	}
	return $mbox;
}


//public
//webmail_admin
function webmail_admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title webmail">'._html_safe(WEBMAIL_ADMINISTRATION)
			."</h1>\n");
	if(($configs = _config_list('webmail')))
	{
		print('<h2 class="title settings">'._html_safe(SETTINGS)
				."</h2>\n");
		$module = 'webmail';
		$action = 'config_update';
		include('./system/config.tpl');
	}
}


//webmail_config_update
function webmail_config_update($args)
{
	global $user_id, $module_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/config.php');
	$keys = array_keys($args);
	foreach($keys as $k)
		if(ereg('^webmail_([a-zA-Z_]+)$', $k, $regs))
			_config_set('webmail', $regs[1], $args[$k], 0);
	header('Location: '._module_link('webmail', 'admin'));
	exit(0);
}


//webmail_default
function webmail_default($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$folder = isset($args['folder']) ? $args['folder'] : 'INBOX';
	if(!($mbox = _webmail_connect($folder)))
	{
		$message = '';
		include('./modules/webmail/login.tpl');
		return;
	}
	$img = 'inbox';
	if($folder == 'Drafts')
		$img = 'drafts';
	else if($folder == 'Sent')
		$img = 'outbox';
	else if($folder == 'Trash')
		$img = 'trash';
	include('./modules/webmail/default.tpl');
	$messages = array();
	$mpp = 20; //FIXME
	$offset = isset($args['page']) && is_numeric($args['page'])
		? $args['page']-1 : 0;
	$offset = $offset * $mpp;
	$cnt = imap_num_msg($mbox);
	$max = min($cnt, $offset + $mpp);
	for($i = $offset; $i < $max; $i++)
	{
		if(!($header = imap_headerinfo($mbox, $i, 80, 80)))
			continue;
		$message = array('link' => _module_link('webmail', 'read',
					imap_uid($mbox, $i), '',
					'folder='._html_safe($folder)));
		$message['icon'] = 'mail_read';
		if($header->Unseen == 'U' || $header->Unseen == 'N')
			$message['icon'] = 'mail_unread';
		else if($header->Answered == 'A')
			$message['icon'] = 'mail_replied';
		$message['icon'] = 'modules/webmail/'.$message['icon'].'.png';
		if($folder == 'Sent')
		{
			$message['name'] = $header->to[0]->personal;
			if(!strlen($message['name']))
				$message['name'] = $header->to[0]->mailbox.'@'
					.$header->to[0]->host;
		}
		else
		{
			$message['name'] = $header->from[0]->personal;
			if(!strlen($message['name']))
				$message['name'] = $header->from[0]->mailbox.'@'
					.$header->from[0]->host;
		}
		$message['subject'] = $header->fetchsubject;
		$message['date'] = date('d/m/Y H:i', $header->udate);
		$messages[] = $message;
	}
	_module('explorer', 'browse', array('entries' => $messages,
			'class' => array('subject' => SUBJECT, 'date' => DATE),
			'view' => 'details'));
	imap_close($mbox);
	return include('./modules/webmail/bottom.tpl');
}


//webmail_folders
function webmail_folders($args)
	//FIXME cache
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	if(!($mbox = _webmail_connect()))
		return include('./modules/webmail/login.tpl');
	if(!($server = _config_get('webmail', 'server')))
		$server = 'localhost';
	$lsub = imap_lsub($mbox, '{'.$server
			.':993/imap/ssl/novalidate-cert}', '*');
	$list = array();
	for($i = 0, $cnt = count($lsub); $i < $cnt; $i++)
	{
		$lsub[$i] = explode('}', $lsub[$i]);
		if($lsub[$i][1] == 'INBOX'
				|| $lsub[$i][1] == 'Drafts'
				|| $lsub[$i][1] == 'Sent'
				|| $lsub[$i][1] == 'Trash')
			continue;
		$list[] = $lsub[$i][1];
	}
	sort($list);
	$level = 0;
	$oldlevel = 0;
	include('./modules/webmail/folders_top.tpl');
	$class = 'lastnode';
	$expand = 0;
	$collapse = 1;
	$hidden = 0;
	$img = 'icons/tree/mlastnode.gif';
	$icon = '../../modules/webmail/inbox_16';
	$folder = '';
	$name = INBOX;
	include('./modules/webmail/tree.tpl');
	$expand = 0;
	$collapse = 0;
	$img = 'icons/tree/node.gif';
	$icon = '../../modules/webmail/drafts_16';
	$folder = '';
	$name = DRAFTS;
	$folder = 'Drafts';
	include('modules/webmail/tree.tpl');
	print("</div>\n");
	$icon = '../../modules/webmail/outbox_16';
	$name = SENT;
	$folder = 'Sent';
	include('./modules/webmail/tree.tpl');
	print("</div>\n");
	$icon = 'delete';
	$name = TRASH;
	$folder = 'Trash';
	include('./modules/webmail/tree.tpl');
	print("</div>\n");
	$icon = 'mime/folder';
	for($i = 0, $cnt = count($list); $i < $cnt; $i++)
	{
		$folder = $list[$i];
		$dirs = explode('.', $folder);
		$level = count($dirs);
		$name = $dirs[$level-1];
		for($j = $oldlevel; $j >= $level; $j--)
		{
			for($k = 0; $k < $oldlevel; $k++)
				print("  ");
			print("</div>\n");
		}
		$expand = 0;
		$level_next = $i+1 == $cnt ? 0
			: count(explode('.', $list[$i+1]));
		$childs = $level_next > $level ? 1 : 0;
		if($i+1 < $cnt && $level_next >= $level)
			$class = 'node';
		else
			$class = 'lastnode';
		if($level >= 1 && $childs)
		{
			$expand = 1;
			$img = 'icons/tree/plastnode.gif';
		}
		else if($childs)
		{
			$collapse = 1;
			$img = 'icons/tree/mlastnode.gif';
		}
		else
			$img = 'icons/tree/'.$class.'.gif';
		$hidden = $level > 1;
		include('./modules/webmail/tree.tpl');
		$oldlevel = $level;
	}
	print("</div>\n</div>\n</div>\n");
	include('./modules/webmail/folders_bottom.tpl');
}


//webmail_login
function webmail_login($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$message = '';
	if(isset($_POST['username']))
	{
		$message = WRONG_PASSWORD;
		$username = stripslashes($_POST['username']);
	}
	include('./modules/webmail/login.tpl');
}


//webmail_logout
function webmail_logout($args)
{
	global $user_id;

	unset($_SESSION['webmail']);
	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	return include('./modules/webmail/logout.tpl');
}


function webmail_read($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$folder = strlen($args['folder']) ? $args['folder'] : 'INBOX';
	if(!($mbox = _webmail_connect($folder)))
		return include('./modules/webmail/login.tpl');
	$headers = imap_fetchheader($mbox, $args['id'],
			FT_UID | FT_PREFETCHTEXT);
	$headers = explode("\n", $headers);
	for($i = 0, $cnt = count($headers); $i < $cnt; $i++)
	{
		//FIXME automate
		$h = explode(': ', $headers[$i]);
		if($h[0] == 'From')
			$from = $h[1];
		else if($h[0] == 'Subject')
			$subject = $h[1];
		else if($h[0] == 'To')
			$to = $h[1];
		else if($h[0] == 'Cc')
			$cc = $h[1];
	}
	$body = imap_body($mbox, stripslashes($args['id']), FT_UID | FT_PEEK);
	imap_close($mbox);
	return include('./modules/webmail/read.tpl');
}


function _system_login()
{
	$_SESSION['webmail']['username'] = stripslashes($_POST['username']);
	$_SESSION['webmail']['password'] = stripslashes($_POST['password']);
	if(!($mbox = _webmail_connect()))
		return;
	imap_close($mbox);
	header('Location: index.php?module=webmail');
	exit(0);
}

function webmail_system($args)
{
	global $title, $html;

	if($_SERVER['REQUEST_METHOD'] == 'POST' && $_POST['action'] == 'login')
		_system_login();
	$title.=' - Webmail';
	if(isset($args['action']) && $args['action'] == 'config_update')
		$html = 0;
	if(isset($args['ajax']) && $args['ajax'] == 1)
	{
		$html = 0;
		require_once('./system/html.php');
		header('Content-type: text/xml');
	}
}

?>
