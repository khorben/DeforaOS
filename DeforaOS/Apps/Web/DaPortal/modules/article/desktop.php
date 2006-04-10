<?php //modules/articles/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

//lang
$text['ARTICLES'] = 'Articles';
$text['SUBMIT'] = 'Submit';
global $lang;
_lang($text);

$title = ARTICLES;
$admin = 1;
$list = 1;
if($user_id)
	$actions = array('submit' => SUBMIT);


?>
