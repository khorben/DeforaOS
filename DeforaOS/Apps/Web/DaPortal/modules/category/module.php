<?php //modules/category/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));


//lang
$text['CATEGORIES_ADMINISTRATION'] = 'Categories administration';
$text['CATEGORIES_LIST'] = 'Categories list';
$text['CATEGORY'] = 'Category';
$text['NEW_CATEGORY'] = ' New category';
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
		$categories[$i]['action'] = 'modify';
		$categories[$i]['icon'] = 'modules/category/icon.png';
		$categories[$i]['thumbnail'] = 'modules/category/icon.png';
		$categories[$i]['enabled'] = $categories[$i]['enabled'] == 't'
			? 'enabled' : 'disabled';
		$categories[$i]['enabled'] = '<img src="icons/16x16/'
				.$categories[$i]['enabled'].'" alt="'
				.$categories[$i]['enabled'].'" title="'
				.($categories[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$categories[$i]['members'] = _sql_single('SELECT COUNT(*)'
				.' FROM daportal_category_content'
				." WHERE category_id='".$categories[$i]['id']
				."';");
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
				'class' => array('enabled' => ENABLED,
					'members' => MEMBERS),
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
	$module = _module_id('category');
	$category = _sql_array('SELECT title'
			.' FROM daportal_content'
			." WHERE content_id='".$args['id']."'"
			." AND module_id='$module'"
			." AND enabled='1';");
	if(!is_array($category) || count($category) != 1)
		return _error('Unable to display category', 1);
	$category = $category[0];
	$title = CATEGORY.' '.$category['title'];
	include('display.tpl');
	$contents = _sql_array('SELECT category_content_id AS id'
			.', module_id, user_id, title AS name'
			.' FROM daportal_category_content, daportal_content'
			.' WHERE daportal_category_content.category_content_id'
			.'=daportal_content.content_id'
			." AND enabled='1'"
			." AND category_id='".$args['id']."';");
	if(!is_array($contents))
		return _error('Unable to display category', 1);
	_module('explorer', 'browse', array('entries' => $contents));
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


function category_modify($args)
{
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	$module = _module_id('category');
	$category = _sql_array('SELECT content_id AS id, title AS name, content'
			.' FROM daportal_content'
			." WHERE content_id='".$args['id']."'"
			." AND module_id='$module'"
			." AND enabled='1';");
	if(!is_array($category) || count($category) != 1)
		return _error('Unable to modify category');
	$category = $category[0];
	$title = MODIFICATION_OF_CATEGORY.' '.$category['title'];
	include('update.tpl');
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
	global $user_id;

	require_once('system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	if(_sql_query('UPDATE daportal_content SET'
			." title='".$args['title']."'"
			.", content='".$args['content']."'"
			." WHERE content_id='".$args['id']."';") == FALSE)
		return _error('Unable to update category');
	category_display(array('id' => $args['id']));
}

?>
