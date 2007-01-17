<?php //modules/comment/desktop.php


$title = 'Comments';
$admin = 1;
$list = 0;

global $user_id;
if($user_id)
	$user = array(array('icon' => 'modules/comment/icon.png',
				'name' => $title,
				'args' => '&user_id='.$user_id));

?>
