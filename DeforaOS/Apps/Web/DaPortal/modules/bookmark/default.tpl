<h1><img src="modules/bookmark/icon.png" alt=""/> <? echo _html_safe(BOOKMARKS); ?></h1>
<p><a href="index.php?module=bookmark&amp;action=new"><? echo _html_safe(NEW_BOOKMARK); ?></a></p>
<? global $user_id; if($user_id) bookmark_list($user_id); else { ?>
<p>You need to <a href="index.php?module=user">login</a> before you may manage bookmarks.</p>
<? } ?>
