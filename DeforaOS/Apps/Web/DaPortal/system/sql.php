<?php //system/sql.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


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
}
if($connection == FALSE)
	_error('Unable to connect to SQL server');

?>
