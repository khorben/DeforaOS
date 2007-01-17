<?php //modules/bookmark/desktop.php


$title = 'Bookmarks';
$admin = 1;
$list = 1;

global $user_id;
if($user_id)
	$user = array(array('icon' => 'modules/bookmark/icon.png',
				'name' => $title,
				'args' => '&user_id='.$user_id));

?>
