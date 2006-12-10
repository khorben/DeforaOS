<?php //system/sql.php



//check url
if(strcmp($_SERVER['SCRIPT_NAME'], $_SERVER['PHP_SELF']) != 0
		|| !ereg('/index.php$', $_SERVER['SCRIPT_NAME']))
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


function _sql_query($query)
{
	return _query($query);
}


//main
$connection = FALSE;
switch($dbtype)
{
	case 'mysql':
		require('./system/sql.mysql.php');
		break;
	case 'pgsql':
		require('./system/sql.pgsql.php');
		break;
	case 'sqlite':
		require('./system/sql.sqlite.php');
		break;
}
if($connection == FALSE)
	exit(_error('Unable to connect to SQL server'));

?>
