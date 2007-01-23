<?php //modules/comment/desktop.php


$title = 'Comments';
$icon = 'comment.png';
$admin = 1;
$list = 0;
global $user_id;
$user = array(array('icon' => 'comment.png', 'name' => $title,
			'args' => '&user_id='.$user_id));

?>
