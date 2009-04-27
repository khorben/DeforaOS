<h1 class="title admin"><?php echo _html_safe(ADMINISTRATION); ?></h1>
<form class="explorer">
	<div class="listing_thumbnails">
<?php foreach($modules as $m) { ?>
		<div class="entry">
			<span class="thumbnail"><img src="<?php echo _html_safe($m['thumbnail']); ?>" alt=""/></span>
			<span class="name"><a href="<?php echo _html_link($m['name'], 'admin'); ?>"><?php echo _html_safe($m['title']); ?></a></span>
		</div>
<?php } ?>
	</div>
	<div style="clear: left">&nbsp;</div>
</div>
