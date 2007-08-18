	<div class="toolbar">
<?php if(isset($args['toolbar'])) { ?>
<?php foreach($toolbar as $t) { if(isset($t['class'])) { ?>
		<a<?php if(isset($t['link'])) { ?> href="<?php echo _html_safe($t['link']); ?>"<?php } if(isset($t['action'])) { ?> onclick="selection_apply(<?php echo $explorer_id; ?>, '<?php echo _html_safe($t['action']); ?>', <?php echo isset($t['confirm']) ? "'"._html_safe($t['confirm'])."'" : '0'; ?>); return false"<?php } if(isset($t['title'])) { ?> title="<?php echo _html_safe($t['title']); ?>"<?php } ?>><div class="icon <?php echo _html_safe($t['class']); ?>"></div></a>
<?php } else { ?>
		<div class="separator"></div>
<?php } } ?>
		<div class="separator"></div>
<?php } ?>
		<a onclick="select_all(<?php echo $explorer_id; ?>); return false" title="<?php echo _html_safe(SELECT_ALL); ?>"><div class="icon select_all"></div></a>
		<div class="separator"></div>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_details'); return false" title="<?php echo _html_safe(LISTING_DETAILS); ?>"><div class="icon listing_details"></div></a>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_list'); return false" title="<?php echo _html_safe(LISTING_LIST); ?>"><div class="icon listing_list"></div></a>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_thumbnails'); return false" title="<?php echo _html_safe(LISTING_THUMBNAILS); ?>"><div class="icon listing_thumbnails"></div></a>
	</div>
