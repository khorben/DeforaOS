<h1><img src="modules/admin/icon.png" alt=""/> Administration</h1>
<style type="text/css"><!-- @import url(modules/explorer/style.css); --></style>
<div class="explorer">
	<div class="listing_thumbnails">
<? foreach($modules as $m) {
if($m['admin'] == 0) continue; ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/<? echo _html_safe_link($m['name']); ?>/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=<? echo _html_safe_link($m['name']); ?>&action=admin"><? echo _html_safe_link($m['title']); ?></a></div>
		</div>
<? } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
