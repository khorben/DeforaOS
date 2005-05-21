<?php
//modules/top/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


function top_admin($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error('Permission denied');
	print('<h1>Top links administration</h1>'."\n");
	$links = _sql_array('SELECT name, link FROM daportal_top'
			.' ORDER BY top_id ASC;');
	if(!is_array($links))
		return _error('Unable to get links');
	_module('explorer', 'browse', array(
			'view' => 'details',
			'class' => array('link' => 'Link'),
			'entries' => $links));
}


function top_default($args)
{
	$links = _sql_array('SELECT name, link FROM daportal_top'
			.' ORDER BY top_id ASC;');
	if(!is_array($links))
		return _error('Unable to get links');
	print("\t\t".'<div class="top">'."\n");
	$sep = '';
	foreach($links as $l)
	{
		print("\t\t\t".$sep.'<a href="'._html_safe($l['link']).'">'
				._html_safe($l['name']).'</a>'."\n");
		$sep = '· ';
	}
	print("\t\t".'</div>'."\n");
}
