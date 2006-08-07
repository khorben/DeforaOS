	<div class="toolbar">
<?php if(isset($args['toolbar'])) { ?>
<?php foreach($args['toolbar'] as $t) { if(isset($t['link'])) { ?>
		<a href="<?php echo _html_safe_link($t['link']); ?>"><img src="<?php echo _html_safe_link($t['icon']); ?>" alt="" title="<?php echo _html_safe($t['title']); ?>"/></a>
<?php } else if(isset($t['action'])) { ?>
		<img src="<?php echo _html_safe_link($t['icon']); ?>" alt="" title="<?php echo _html_safe($t['title']); ?>" onclick="selection_apply(<?php echo $explorer_id; ?>, '<?php echo _html_safe($t['action']); ?>', <?php echo isset($t['confirm']) ? "'"._html_safe($t['confirm'])."'" : '0'; ?>)"/>
<?php } else { ?>
		<div class="separator"></div>
<?php } } ?>
		<div class="separator"></div>
<?php } ?>
		<img src="modules/explorer/select_all.png" alt="select all" title="<?php echo _html_safe(SELECT_ALL); ?>" onclick="select_all(<?php echo $explorer_id; ?>)"/>
		<div class="separator"></div>
		<img src="modules/explorer/details.png" alt="details" title="<?php echo _html_safe(LISTING_DETAILS); ?>" onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_details')"/>
		<img src="modules/explorer/list.png" alt="list" title="<?php echo _html_safe(LISTING_LIST); ?>" onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_list')"/>
		<img src="modules/explorer/thumbnails.png" alt="thumbnails" title="<?php echo _html_safe(LISTING_THUMBNAILS); ?>" onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_thumbnails')"/>
	</div>
