<?php //system/config.php



function _config_get($module, $name)
{
	if(!($module_id = _module_id($module)))
		return FALSE;
	return _sql_single('SELECT value FROM daportal_config'
			.' WHERE daportal_config.module_id='."'$module_id'"
			.' AND daportal_config.name='."'$name'");
}


function _config_list($module)
{
	if(!($module_id = _module_id($module)))
		return FALSE;
	return _sql_array('SELECT daportal_config.name AS name, value'
			.' FROM daportal_config'
			." WHERE daportal_config.module_id='".$module_id."'"
			.' ORDER BY name ASC');
}


function _config_set($module, $name, $value, $overwrite = FALSE)
{
	if(($module_id = _sql_single('SELECT daportal_config.module_id'
			.' FROM daportal_config, daportal_module'
			.' WHERE daportal_config.module_id'
			.'=daportal_module.module_id'
			." AND daportal_module.name='$module'"
			." AND daportal_config.name='$name'")) == FALSE)
	{
		if($overwrite != 0)
			return FALSE;
		return _sql_query('INSERT INTO daportal_config'
				.' (module_id, name, value)'
				." VALUES ('$module_id', '$name', '$value')");
	}
	return _sql_query("UPDATE daportal_config SET value='$value'"
			." WHERE module_id='$module_id' AND name='$name'");
}

?>
