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
