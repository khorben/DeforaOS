<?php global $user_id; if($user_id) bookmark_list(array('user_id' => $user_id)); else { ?>
<h1 class="title bookmark"><?php echo _html_safe(BOOKMARKS); ?></h1>
<p>You need to <a href="index.php?module=user">login</a> before you may manage bookmarks.</p>
<?php } ?>
