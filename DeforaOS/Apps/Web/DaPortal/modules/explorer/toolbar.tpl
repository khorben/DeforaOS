	<div class="toolbar">
<?php if(isset($args['toolbar'])) { ?>
<?php foreach($toolbar as $t) { if(isset($t['class'])) { ?>
		<a<?php if(isset($t['link'])) { ?> href="<?php echo _html_safe($t['link']); ?>"<?php } if(isset($t['action'])) { ?> onclick="selection_apply(<?php echo $explorer_id; ?>, '<?php echo _html_safe($t['action']); ?>', <?php echo isset($t['confirm']) ? "'"._html_safe($t['confirm'])."'" : '0'; ?>)"<?php } if(isset($t['title'])) { ?> title="<?php echo _html_safe($t['title']); ?>"<?php } if(isset($t['onclick'])) { ?> onclick="<?php echo _html_safe($t['onclick']); ?>"<?php } ?>><?php echo _html_icon($t['class']); ?></a>
<?php } else { ?>
		<span class="separator"></span>
<?php } } ?>
		<span class="separator"></span>
<?php } ?>
		<a onclick="return select_all(<?php echo $explorer_id; ?>)" title="<?php echo _html_safe(SELECT_ALL); ?>"><?php echo _html_icon('select-all'); ?></a>
		<span class="separator"></span>
		<a onclick="return change_class('explorer_<?php echo $explorer_id; ?>', 'listing_details')" title="<?php echo _html_safe(LISTING_DETAILS); ?>"><?php echo _html_icon('listing_details'); ?></a>
		<a onclick="return change_class('explorer_<?php echo $explorer_id; ?>', 'listing_list')" title="<?php echo _html_safe(LISTING_LIST); ?>"><?php echo _html_icon('listing_list'); ?></a>
		<a onclick="return change_class('explorer_<?php echo $explorer_id; ?>', 'listing_thumbnails')" title="<?php echo _html_safe(LISTING_THUMBNAILS); ?>"><?php echo _html_icon('listing_thumbnails'); ?></a>
	</div>
