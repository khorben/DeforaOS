<h1 class="admin">Administration</h1>
<style type="text/css"><!-- @import url('css/explorer.css'); --></style>
<div class="explorer">
	<div class="listing_thumbnails">
<?php foreach($modules as $m) {
if($m['admin'] == 0) continue; ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/<?php echo _html_safe_link($m['name']); ?>/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=<?php echo _html_safe_link($m['name']); ?>&action=admin"><?php echo _html_safe_link($m['title']); ?></a></div>
		</div>
<?php } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
