<?php //system/sql.pgsql.php



//check url
if(strcmp($_SERVER['SCRIPT_NAME'], $_SERVER['PHP_SELF']) != 0
		|| !ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));

define('SQL_TRUE', 't');
define('SQL_FALSE', 'f');


function _query($query)
{
	global $_pg_res;

	_info($query);
	$_pg_res = pg_query($query);
	if(strlen($str = pg_last_error()))
		_error($str, 0);
	return $_pg_res;
}


function _sql_array($query)
{
	if(($res = _query($query)) == FALSE)
		return FALSE;
	for($array = array(); ($a = pg_fetch_array($res)) != FALSE;
			$array[] = $a);
	return $array;
}


function _sql_enum($table, $field)
{
	$res = _sql_array('SELECT pg_catalog.pg_get_constraintdef(r.oid)'
			.' AS constraint FROM pg_catalog.pg_class c'
			.', pg_catalog.pg_constraint r'
			." WHERE c.oid=r.conrelid AND c.relname='".$table."'"
			." AND conname='".$table.'_'.$field."';");
	$res = explode("'", $res[0]['constraint']);
	$str = array();
	for($i = 1, $cnt = count($res); $i < $cnt; $i+=2)
		$str[] = $res[$i];
	return $str;
}


function _sql_id($table, $field)
{
	return _sql_single("SELECT currval('".$table."_".$field."_seq');");
}


function _sql_offset($offset, $limit)
{
	return 'OFFSET '.$offset.' LIMIT '.$limit;
}


function _sql_single($query)
{
	if(($res = _sql_query($query)) == FALSE)
		return FALSE;
	if(pg_num_rows($res) != 1 || pg_num_fields($res) != 1)
		return FALSE;
	return pg_fetch_result($res, 0, 0);
}


//main
global $dbuser, $dbpassword;
$connection = pg_connect("user='$dbuser' password='$dbpassword'");

?>
