<?php
//modules/explorer/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function _explorer(&$args)
{
	static $explorer_id = 0;

	$explorer_id++;
	//FIXME default values table
	include('top.tpl');
	if(!isset($args['toolbar']) || $args['toolbar'])
		include('toolbar.tpl');
	$view = isset($args['view']) ? $args['view'] : 'thumbnails';
	include('header.tpl');
	$i = 0;
	foreach($args['entries'] as $entry)
	{
		$i++;
		$link = _explorer_link($entry);
		$link_end = strlen($link) ? '</a>' : '';
		include('entry.tpl');
	}
	include('bottom.tpl');
}

function _explorer_link(&$entry)
{
	if(isset($entry['link']))
		return '<a href="'._html_safe($entry['link']).'">';
	else if(!isset($entry['link']) && isset($entry['module'])
			&& isset($entry['action']))
	{
		$link = '<a href="'._html_safe('index.php'
				.'?module='.$entry['module']
				.'&action='.$entry['action']);
		if(isset($entry['id']))
			$link.=_html_safe('&id='.$entry['id']);
		return $link.'">';
	}
	return '';
}


function explorer_browse($args)
{
	foreach($args['entries'] as $entry)
		foreach($args['columns'] as $key)
			$entry[$key] = _html_safe($entry[$key]);
	return _explorer($args);
}


function explorer_browse_trusted($args)
{
	return _explorer($args);
}
