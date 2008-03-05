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



//lang
$text = array();
$text['LISTING_DETAILS'] = 'Details';
$text['LISTING_LIST'] = 'List';
$text['LISTING_THUMBNAILS'] = 'Thumbnails';
$text['SELECT_ALL'] = 'Select all';
global $lang;
if($lang == 'de')
	$text['SELECT_ALL'] = 'Alles markieren';
else if($lang == 'fr')
{
	$text['LISTING_DETAILS'] = 'Détails';
	$text['LISTING_LIST'] = 'Liste';
	$text['LISTING_THUMBNAILS'] = 'Vignettes';
	$text['SELECT_ALL'] = 'Tout sélectionner';
}
_lang($text);


function _explorer(&$args)
{
	static $explorer_id = 0;

	$explorer_id++;
	$module = isset($args['module']) ? $args['module'] : '';
	$action = isset($args['action']) ? $args['action'] : '';
	$id = isset($args['id']) ? $args['id'] : '';
	include('./modules/explorer/top.tpl');
	if(!isset($args['toolbar']) || is_array($args['toolbar']))
	{
		$toolbar = isset($args['toolbar']) ? $args['toolbar'] : array();
		include('./modules/explorer/toolbar.tpl');
	}
	$view = isset($args['view']) ? $args['view'] : (isset($_SESSION['view'])
			? $_SESSION['view'] : 'thumbnails');
	$class = isset($args['class']) && is_array($args['class'])
		? array_keys($args['class']) : array();
	include('./modules/explorer/header.tpl');
	$i = 0;
	foreach($args['entries'] as $entry)
	{
		$i++;
		$link = _explorer_link($entry);
		$link_end = strlen($link) ? '</a>' : '';
		include('./modules/explorer/entry.tpl');
	}
	include('./modules/explorer/bottom.tpl');
}

function _explorer_link(&$entry)
{
	if(isset($entry['link']))
	{
		if(isset($entry['title']))
			return '<a href="'._html_safe($entry['link'])
				.'" title="'._html_safe($entry['title']).'">';
		return '<a href="'._html_safe($entry['link']).'">';
	}
	else if(!isset($entry['link']) && isset($entry['module'])
			&& isset($entry['action']))
	{
		$link = '<a href="'._html_link($entry['module'],
			isset($entry['action']) ? $entry['action'] : FALSE,
			isset($entry['id']) ? $entry['id'] : FALSE,
			isset($entry['tag']) ? $entry['tag'] : FALSE,
			isset($entry['args']) ? $entry['args'] : FALSE);
		$link.='"';
		if(isset($entry['title']))
			$link.=' title="'._html_safe($entry['title']).'"';
		return $link.'>';
	}
	return '';
}

function _explorer_sort($module, $action, $args, $class, $sort, $name)
{
	if($class == $sort)
		return print(' sort">'._html_safe($name));
	$link = _html_link($module, $action, FALSE, FALSE, $args);
	$link .= (strlen($args) ? '&' : '?').'sort='.$class; /* XXX ugly */
	print('"><a href="'.$link.'">'._html_safe($name).'</a>');
}


function explorer_apply($args)
{
	global $error;

	if(isset($error) && strlen($error))
		_error($error);
	if(!isset($args['link_module']))
		return;
	_module($args['link_module'], isset($args['link_action'])
			? $args['link_action'] : 'default',
			isset($args['link_id']) ? $args['link_id'] : FALSE);
}


function explorer_browse($args)
{
	for($i = 0, $cnt = count($args['entries']); $i < $cnt; $i++)
	{
		$args['entries'][$i]['name'] = _html_safe(
				$args['entries'][$i]['name']);
		if(!isset($args['class']) || !is_array($args['class']))
			continue;
		$classes = array_keys($args['class']);
		foreach($classes as $c)
			$args['entries'][$i][$c] = _html_safe(
					$args['entries'][$i][$c]);
	}
	return _explorer($args);
}


function explorer_browse_trusted($args)
{
	return _explorer($args);
}


function explorer_system($args)
{
	global $error;

	if(!isset($args['action'])
			|| $_SERVER['REQUEST_METHOD'] != 'POST')
		return;
	switch($args['action'])
	{
		case 'apply':
			$error = _system_apply($args);
			break;
	}
}

function _system_apply($args)
{
	if(!isset($args['link_module']) || !isset($args['link_action']))
		return 'Need a module and action to link to';
	if(!isset($args['apply']))
		return 'Need an action to apply';
	$action = $args['apply'];
	if(!ereg('^[a-z0-9_]{1,30}$', $action))
		return 'Invalid action to apply';
	$keys = array_keys($args);
	foreach($keys as $k)
	{
		if(!ereg('^entry_[0-9]+_[0-9]+$', $k)
				|| !isset($args[$k.'_module'])
				|| !isset($args[$k.'_id']))
			continue;
		$module = $args[$k.'_module'];
		$id = $args[$k.'_id'];
		if(!strlen($module) || !strlen($action) || !strlen($id))
			continue;
		$params = array('id' => $id);
		if(isset($args[$k.'_args']))
		{
			$extras = explode(';', $args[$k.'_args']);
			foreach($extras as $e)
			{
				$extra = explode('=', $e);
				$params[$extra[0]] = $extra[1];
			}
		}
		_module($module, $action, $params);
	}
	$link = _module_link($args['link_module'], $args['link_action'],
			isset($args['link_id']) && is_numeric($args['link_id'])
			? $args['link_id'] : FALSE);
	header('Location: '.$link);
	exit(0);
}

?>
