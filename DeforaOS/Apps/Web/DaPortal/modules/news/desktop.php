<?php //modules/news/desktop.php



$title = 'News';
$icon = 'news.png';
$admin = 1;
$list = 1;
global $user_id;
if($user_id)
{
	$actions = array('submit' => SUBMIT);
	$user = array(array('icon' => 'news.png', 'name' => $title,
				'action' => 'list',
				'args' => '&user_id='.$user_id));
}


?>
