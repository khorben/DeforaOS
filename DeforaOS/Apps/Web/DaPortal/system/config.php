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



function _config_get($module, $name)
{
	if(!($module_id = _module_id($module)))
		return FALSE;
	$res = _sql_array('SELECT type, value_bool, value_int, value_string'
			.' FROM daportal_config'
			.' WHERE daportal_config.module_id='."'$module_id'"
			.' AND daportal_config.name='."'$name'");
	if(!is_array($res) || count($res) != 1)
		return FALSE;
	return $res[0]['value_'.$res[0]['type']];
}


function _config_list($module)
{
	if(!($module_id = _module_id($module)))
		return FALSE;
	$res = _sql_array('SELECT type, name, value_bool, value_int'
			.', value_string FROM daportal_config'
			." WHERE daportal_config.module_id='".$module_id."'"
			.' ORDER BY name ASC');
	if(!is_array($res))
		return FALSE;
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		$res[$i]['value'] = $res[$i]['value_'.$res[$i]['type']];
	return $res;
}


function _config_set($module, $name, $value, $overwrite = FALSE)
{
	if($_SERVER['REQUEST_METHOD'] != 'POST')
		return FALSE;
	$res = _sql_array('SELECT daportal_config.module_id AS module_id'
			.', type FROM daportal_config, daportal_module'
			.' WHERE daportal_config.module_id'
			.'=daportal_module.module_id'
			." AND daportal_module.name='$module'"
			." AND daportal_config.name='$name'");
	if(!is_array($res))
	{
		if($overwrite != FALSE)
			return FALSE;
		return _sql_query('INSERT INTO daportal_config'
				.' (module_id, type, name, value)'
				." VALUES ('$module_id', 'string', '$name'"
				.", '$value')");
	}
	if(count($res) != 1)
		return FALSE;
	$res = $res[0];
	if($res['type'] == 'bool')
		$value = ($value === SQL_FALSE) ? SQL_FALSE : SQL_TRUE;
	return _sql_query('UPDATE daportal_config SET value_'.$res['type']
			."='$value' WHERE module_id='".$res['module_id']."'"
			." AND name='$name'");
}

?>
