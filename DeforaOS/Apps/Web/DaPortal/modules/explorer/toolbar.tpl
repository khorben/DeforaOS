	<div class="toolbar">
<?php if(isset($args['toolbar'])) { ?>
<?php foreach($toolbar as $t) { if(isset($t['class'])) { ?>
		<a<?php if(isset($t['link'])) { ?> href="<?php echo _html_safe($t['link']); ?>"<?php } if(isset($t['action'])) { ?> onclick="selection_apply(<?php echo $explorer_id; ?>, '<?php echo _html_safe($t['action']); ?>', <?php echo isset($t['confirm']) ? "'"._html_safe($t['confirm'])."'" : '0'; ?>)"<?php } if(isset($t['title'])) { ?> title="<?php echo _html_safe($t['title']); ?>"<?php } if(isset($t['onclick'])) { ?> onclick="<?php echo _html_safe($t['onclick']); ?>"<?php } ?>><span class="icon <?php echo _html_safe($t['class']); ?>"></span></a>
<?php } else { ?>
		<span class="separator"></span>
<?php } } ?>
		<span class="separator"></span>
<?php } ?>
		<a onclick="select_all(<?php echo $explorer_id; ?>)" title="<?php echo _html_safe(SELECT_ALL); ?>"><span class="icon select_all"></span></a>
		<span class="separator"></span>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_details')" title="<?php echo _html_safe(LISTING_DETAILS); ?>"><span class="icon listing_details"></span></a>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_list')" title="<?php echo _html_safe(LISTING_LIST); ?>"><span class="icon listing_list"></span></a>
		<a onclick="change_class('explorer_<?php echo $explorer_id; ?>', 'listing_thumbnails')" title="<?php echo _html_safe(LISTING_THUMBNAILS); ?>"><span class="icon listing_thumbnails"></span></a>
	</div>
