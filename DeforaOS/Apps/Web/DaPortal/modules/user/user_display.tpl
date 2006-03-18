<h1><img src="modules/user/user.png" alt=""/> <? echo _html_safe($user['username']); ?></h1>
<style type="text/css"><!-- @import url(modules/explorer/style.css); --></style>
<div class="explorer">
	<div class="listing_thumbnails">
<? global $user_id; if(_module_id('bookmark') && $user_id != 0) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/bookmark/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=bookmark&amp;user_id=<? echo _html_safe_link($user['user_id']); ?>">Bookmarks</a></div>
		</div>
<? } ?>
<? if(_module_id('news')) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/news/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=news&amp;user_id=<? echo _html_safe_link($user['user_id']); ?>">News</a></div>
		</div>
<? } ?>
<? if(_module_id('comment')) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/comment/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=comment&amp;user_id=<? echo _html_safe_link($user['user_id']); ?>">Comments</a></div>
		</div>
<? } ?>
<? if(_module_id('project')) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/project/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=project&amp;action=list&amp;user_id=<? echo _html_safe_link($user['user_id']); ?>">Projects</a></div>
		</div>
		<div class="entry">
			<div class="thumbnail"><img src="modules/project/bug.png" alt=""/></div>
			<div class="name"><a href="index.php?module=project&amp;action=bug_list&amp;user_id=<? echo _html_safe_link($user['user_id']); ?>">Bug reports</a></div>
		</div>
<? } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
