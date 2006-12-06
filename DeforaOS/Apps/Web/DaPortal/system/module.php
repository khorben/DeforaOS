<?php //system/module.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _module($module = '', $action = '', $args = FALSE)
{
	global $module_id, $module_name, $html;

	if(strlen($module))
	{
		if(!is_array($args))
			$args = array();
	}
	else if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		$module = isset($_GET['module']) ? $_GET['module'] : '';
		if(!strlen($action) || $args == FALSE)
			$args =& $_GET;
		$action = strlen($action) ? $action : (isset($_GET['action'])
				? $_GET['action'] : '');
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
	{
		$module = isset($_POST['module']) ? $_POST['module'] : '';
		if(!strlen($action) || $args == FALSE)
			$args =& $_POST;
		$action = strlen($action) ? $action : $_POST['action'];
	}
	else
		return _error('Invalid module request', 0);
	if(!strlen($action) || !ereg('^[a-z0-9_]{1,30}$', $action))
		$action = 'default';
	if(!ereg('^[a-z]{1,10}$', $module) || ($id = _module_id($module)) == 0)
		return _error('Invalid module', 0);
	$module_id = $id;
	$module_name = $module;
	if(!include_once('./modules/'.$module_name.'/module.php'))
		return _error('Could not include module "'.$module_name.'"', 0);
	$function = $module_name.'_'.$action;
	if(!function_exists($function))
		return _warning('Unknown action "'.$action.'" for module "'
				.$module_name.'"');
	_info('Module "'.$module_name.'", action "'.$action.'"');
	return call_user_func_array($function, array($args));
}


function _module_id($name)
{
	static $cache = array();
	global $user_id;

	if(isset($cache[$name]))
		return $cache[$name];
	require_once('./system/user.php');
	$enabled = " AND enabled='1'";
	if(_user_admin($user_id))
		$enabled = '';
	if(($id = _sql_single('SELECT module_id FROM daportal_module'
			." WHERE name='$name'$enabled;")) == FALSE)
		return 0;
	$cache[$name] = $id;
	return $id;
}


function _module_desktop()
	//FIXME fetch an option from a module desktop file
{
}

?>
