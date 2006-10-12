<?php //modules/explorer/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
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
		include('./modules/explorer/toolbar.tpl');
	}
	$view = isset($args['view']) ? $args['view'] : 'thumbnails';
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
			return '<a href="'._html_safe_link($entry['link'])
				.'" title="'._html_safe($entry['title']).'">';
		return '<a href="'._html_safe_link($entry['link']).'">';
	}
	else if(!isset($entry['link']) && isset($entry['module'])
			&& isset($entry['action']))
	{
		$link = '<a href="index.php?module='
			._html_safe_link($entry['module']).'&amp;action='
			._html_safe_link($entry['action']);
		if(isset($entry['id']))
			$link.='&amp;id='._html_safe_link($entry['id']);
		if(isset($entry['args']))
			$link.=_html_safe($entry['args']);
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
	{
		print(_html_safe($name));
		print('<img src="modules/explorer/sort.png" alt="sort"/>');
		return;
	}
	print('<a href="index.php?');
	$link = 'module='.$module.'&action='.$action.$args
		.'&sort='.$class;
	print(_html_safe($link).'">'._html_safe($name).'</a>');
}


function explorer_apply($args)
{
	if(!strlen($args['link_module']) || !strlen($args['link_action']))
		return _error('Need a module and action to link to');
	if(!ereg('^[a-z0-9_]{1,30}$', $args['apply']))
		return _error('Invalid action to apply');
	$keys = array_keys($args);
	foreach($keys as $k)
	{
		if(!ereg('^entry_[0-9]+_[0-9]+$', $k))
			continue;
		$module = $args[$k.'_module'];
		$action = $args['apply'];
		$id = $args[$k.'_id'];
		if(!strlen($module) || !strlen($action) || !strlen($id))
			continue;
		$params = array('id' => $id);
		if(isset($args[$k.'_args']))
		{
			$extras = split(';', $args[$k.'_args']);
			foreach($extras as $e)
			{
				$extra = split('=', $e);
				$params[$extra[0]] = $extra[1];
			}
		}
		_module($module, $action, $params);
	}
	$link = 'index.php?module='.$args['link_module']
			.'&action='.$args['link_action'];
	if(is_numeric($args['link_id']))
		$link.='&id='.$args['link_id'];
	header('Location: '.$link);
	exit(0);
}


function explorer_browse(&$args)
{
	for($i = 0, $ic = count($args['entries']); $i < $ic; $i++)
	{
		$args['entries'][$i]['name'] = _html_safe_link(
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


function explorer_browse_trusted(&$args)
{
	return _explorer($args);
}


function explorer_system($args)
{
	global $html;

	if($args['action'] == 'apply')
		$html = 0;
}

?>
