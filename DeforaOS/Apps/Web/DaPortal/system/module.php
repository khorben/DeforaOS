<?php



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
		$module = $_GET['module'];
		$args = strlen($action) ? $args : $_GET;
		$action = strlen($action) ? $action : $_GET['action'];
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
	{
		$module = $_POST['module'];
		$args = strlen($action) ? $args : $_POST;
		$action = strlen($action) ? $action : $_POST['action'];
	}
	else
		return _error('Invalid module request');
	if(!strlen($action))
		$action = 'default';
	if(($id = _sql_single('SELECT module_id FROM daportal_module'
			." WHERE name='".$module."' AND enabled='1'")) == FALSE)
		return _error('Invalid module');
	$module_id = $id;
	$module_name = $module;
	if(!include_once('modules/'.$module_name.'/module.php'))
		return _error('Could not include module "'.$module_name.'"');
	$function = $module_name.'_'.$action;
	if(!function_exists($function))
		return _warning('Unknown action "'.$action.'" for module "'
				.$module_name.'"');
	_info('Module "'.$module_name.'", action "'.$action.'"');
	$css = 'modules/'.$module_name.'/style.css';
	if($html && file_exists($css))
		print("\t\t\t".'<style type="text/css"><!-- @import url('
				.$css.'); --></style>'."\n");
	return call_user_func_array($function, array($args));
}

_module('', 'system');

?>
