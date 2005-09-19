<?php
//system/sql.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _query($query)
{
	global $dbtype, $_pg_res;

	_info($query);
	switch($dbtype)
	{
		case 'mysql':
			return @mysql_query($query);
		case 'pgsql':
			$_pg_res = @pg_query($query);
			if(strlen($str = @pg_last_error()))
				_error($str, 0);
			return $_pg_res;
	}
}


function _sql_array($query)
{
	global $dbtype;

	if(($res = _query($query)) == FALSE)
		return FALSE;
	$array = array();
	for(;; $array[] = $a)
	{
		switch($dbtype)
		{
			case 'mysql':
				$a = mysql_fetch_array($res);
				break;
			case 'pgsql':
				$a = pg_fetch_array($res);
				break;
		}
		if($a == FALSE)
			break;
	}
	return $array;
}


function _sql_enum($table, $field)
{
	$str = sql_array('SHOW COLUMNS FROM '.$table." LIKE '$field';");
	$str = ereg_replace("^enum\('(.*)'\)$", '\1', $str[0]['Type']);
	return split("[']?,[']?", $str);
}


function _sql_id($table, $field)
{
	global $dbtype, $_pg_res;

	switch($dbtype)
	{
		case 'mysql':
			return mysql_insert_id();
		case 'pgsql':
			return _sql_single("SELECT currval('".$table
					."_".$field."_seq');");
	}
}


function _sql_query($query)
{
	return _query($query);
}


function _sql_single($query)
{
	global $dbtype;

	$res = _sql_query($query);
	if($res == FALSE)
		return FALSE;
	switch($dbtype)
	{
		case 'mysql':
			if(mysql_num_rows($res) != 1 || mysql_num_fields($res) != 1)
				return FALSE;
			return mysql_result($res, 0);
		case 'pgsql':
			if(pg_num_rows($res) != 1 || pg_num_fields($res) != 1)
				return FALSE;
			return pg_fetch_result($res, 0, 0);
	}
}


//main
global $dbtype, $dbhost, $dbuser, $dbpassword, $dbname;
switch($dbtype)
{
	case 'mysql':
		$connection = @mysql_connect($dbhost, $dbuser, $dbpassword)
				&& mysql_select_db($dbname, $connection);
		break;
	case 'pgsql':
		$connection = @pg_connect("user='$dbuser' password='$dbpassword'");
		break;
}
if($connection == FALSE)
	_error('Unable to connect to SQL server');

?>
