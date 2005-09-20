<h1><img src="modules/user/home.png" alt=""/> <? echo _html_safe($user_name); ?>'s page</h1>
<style type="text/css"><!-- @import url(modules/explorer/style.css); --></style>
<div class="explorer">
	<div class="listing_thumbnails">
<? global $user_id; require_once('system/user.php'); if(_user_admin($user_id)) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/admin/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=admin">Administration</a></div>
		</div>
<? } ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/admin/content.png" alt=""/></div>
			<div class="name"><a href="index.php?module=user&amp;id=<? echo _html_safe($user_id); ?>">My Contents</a></div>
		</div>
		<div class="entry">
			<div class="thumbnail"><img src="modules/user/logout.png" alt=""/></div>
			<div class="name"><a href="index.php?module=user&amp;action=logout"><? echo _html_safe(LOGOUT); ?></a></div>
		</div>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
