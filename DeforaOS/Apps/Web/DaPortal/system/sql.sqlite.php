<?php //system/sql.sqlite.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));

define('SQL_TRUE', 1);
define('SQL_FALSE', 0);


function _query($query)
{
	global $connection;

	_info($query);
	if(($res = sqlite_query($connection, $query)) == FALSE)
		_error(sqlite_error_string(sqlite_last_error($connection)), 0);
	return $res;
}


function _sql_array($query)
{
	if(($res = _query($query)) == FALSE)
		return FALSE;
	for($array = array(); ($a = sqlite_fetch_array($res)) != FALSE;
			$array[] = $a);
	return $array;
}


function _sql_enum($query)
{
	/* FIXME */
	return array();
}


function _sql_id($table, $field)
{
	global $connection;

	return sqlite_last_insert_rowid($connection);
}


function _sql_offset($offset, $limit)
{
	return 'LIMIT '.$limit.' OFFSET '.$offset;
}


function _sql_single($query)
{
	if(($res = _query($query)) == FALSE)
		return FALSE;
	return sqlite_fetch_single($res);
}


//main
global $dbname;
$error = '';
if(($connection = sqlite_open($dbname, 0666, $error)) == FALSE)
	_error($error, 0);

?>
