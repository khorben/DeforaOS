<?php



//check url
if(!eregi("index.php", $_SERVER["REQUEST_URI"]))
{
	header("Location: index.php");
	exit(0);
}

include("install.php");


?>
