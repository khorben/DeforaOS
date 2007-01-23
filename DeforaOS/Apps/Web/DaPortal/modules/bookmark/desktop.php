<?php //modules/bookmark/desktop.php


$title = 'Bookmarks';
$icon = 'bookmark.png';
$admin = 1;
$list = 1;

global $user_id;
if($user_id)
	$user = array(array('icon' => 'bookmark.png', 'name' => $title,
				'args' => '&user_id='.$user_id));

?>
