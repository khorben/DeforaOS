<?php //modules/category/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['CATEGORIES_ADMINISTRATION'] = 'Categories administration';
$text['CATEGORIES_LIST'] = 'Categories list';
global $lang;
_lang($text);


function category_admin($args)
{
	print('<h1><img src="modules/category/icon.png" alt=""/> '
			.CATEGORIES_ADMINISTRATION.'</h1>'."\n");
	$module_id = _module_id('category');
	$categories = _sql_array('SELECT content_id AS id, title AS name'
			.', enabled, content AS description'
			.' FROM daportal_content'
			." WHERE module_id='$module_id';");
	if(!is_array($categories))
		return _error('Could not list categories');
	$count = count($categories);
	for($i = 0; $i < $count; $i++)
	{
		$categories[$i]['module'] = 'category';
		$categories[$i]['action'] = 'default';
		$categories[$i]['icon'] = 'modules/category/icon.png';
		$categories[$i]['thumbnail'] = 'modules/category/icon.png';
		$categories[$i]['enabled'] = $categories[$i]['enabled'] == 't'
			? 'enabled' : 'disabled';
		$categories[$i]['enabled'] = '<img src="icons/16x16/'
				.$categories[$i]['enabled'].'" alt="'
				.$categories[$i]['enabled'].'" title="'
				.($categories[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$categories[$i]['apply_module'] = 'category';
		$categories[$i]['apply_id'] = $categories[$i]['id'];
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CATEGORY,
			'icon' => 'modules/category/icon.png',
			'link' => 'index.php?module=category&action=new');
	$toolbar[] = array();
	$toolbar[] = array('title' => ENABLE,
			'icon' => 'icons/16x16/enabled.png',
			'action' => 'enable');
	$toolbar[] = array('title' => DISABLE,
			'icon' => 'icons/16x16/disabled.png',
			'action' => 'disable');
	_module('explorer', 'browse_trusted', array('entries' => $categories,
				'class' => array('enabled' => ENABLED),
				'toolbar' => $toolbar,
				'view' => 'details',
				'module' => 'category',
				'action' => 'admin'));
}


function category_default($args)
{
	if(isset($args['id']))
		return category_display($args);
	return category_list($args);
}


function category_disable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	_content_disable($args['id']);
}


function category_display($args)
{
	/* FIXME */
	include('display.tpl');
}


function category_enable($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	_content_enable($args['id']);
}


function category_insert($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	require_once('system/content.php');
	if(($id = _content_insert($args['title'], $args['content'], 1))
			== FALSE)
		return _error('Unable to insert category');
	category_display(array('id' => $id));
}


function category_list($args)
{
	print('<h1><img src="modules/category/icon.png" alt=""/> '
			.CATEGORIES_LIST.'</h1>'."\n");
	$module_id = _module_id('category');
	$categories = _sql_array('SELECT content_id AS id, title AS name'
			.', content AS description'
			.' FROM daportal_content'
			." WHERE enabled='1' AND module_id='$module_id';");
	if(!is_array($categories))
		return _error('Could not list categories');
	$count = count($categories);
	for($i = 0; $i < $count; $i++)
	{
		$categories[$i]['module'] = 'category';
		$categories[$i]['action'] = 'default';
		$categories[$i]['icon'] = 'modules/category/icon.png';
		$categories[$i]['thumbnail'] = 'modules/category/icon.png';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_CATEGORY,
			'icon' => 'modules/category/icon.png',
			'link' => 'index.php?module=category&action=new');
	_module('explorer', 'browse', array('entries' => $categories,
				'toolbar' => $toolbar));
}


function category_new($args)
{
	global $user_id;

	if($user_id == 0)
		return _error(PERMISSION_DENIED);
	$title = NEW_CATEGORY;
	include('update.tpl');
}


function category_update($args)
{
	/* FIXME */
}

?>
