<h1 class="title admin"><?php echo _html_safe(ADMINISTRATION); ?></h1>
<div class="explorer">
	<div class="listing_thumbnails">
<?php foreach($modules as $m) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="modules/<?php echo _html_safe_link($m['name']); ?>/icon.png" alt=""/></div>
			<div class="name"><a href="index.php?module=<?php echo _html_safe_link($m['name']); ?>&action=admin"><?php echo _html_safe_link($m['title']); ?></a></div>
		</div>
<?php } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
