<?php //modules/articles/desktop.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../../index.php'));

//lang
$text['ARTICLES'] = 'Articles';
$text['SUBMIT'] = 'Submit';
global $lang, $user_id;
_lang($text);

$title = ARTICLES;
$icon = 'article.png';
$admin = 1;
$list = 1;
$user = array(array('icon' => 'article.png', 'name' => ARTICLES,
			'action' => 'list', 'args' => '&user_id='.$user_id));
if($user_id)
	$actions = array('submit' => SUBMIT);


?>
