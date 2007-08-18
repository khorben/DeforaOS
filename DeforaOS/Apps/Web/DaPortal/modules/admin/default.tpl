<h1 class="title admin"><?php echo _html_safe(ADMINISTRATION); ?></h1>
<div class="explorer">
	<div class="listing_thumbnails">
<?php foreach($modules as $m) { ?>
		<div class="entry">
			<div class="thumbnail"><img src="<?php echo _html_safe($m['thumbnail']); ?>" alt=""/></div>
			<div class="name"><a href="<?php echo _html_link($m['name'], 'admin'); ?>"><?php echo _html_safe($m['title']); ?></a></div>
		</div>
<?php } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
