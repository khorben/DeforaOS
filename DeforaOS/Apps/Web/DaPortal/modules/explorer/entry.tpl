		<div class="entry" onclick="entry_click(<?php echo $explorer_id; ?>, <?php echo $i; ?>, event)">
			<input class="hidden" type="checkbox" name="entry_<?php echo $explorer_id; ?>_<?php echo $i; ?>"/>
<?php if(isset($entry['apply_module']) && isset($entry['apply_id'])) { ?>
			<input type="hidden" name="entry_<?php echo $explorer_id.'_'.$i; ?>_module" value="<?php echo _html_safe($entry['apply_module']); ?>"/>
			<input type="hidden" name="entry_<?php echo $explorer_id.'_'.$i; ?>_id" value="<?php echo _html_safe($entry['apply_id']); ?>"/>
<?php if(isset($entry['apply_args'])) { ?>
			<input type="hidden" name="entry_<?php echo $explorer_id.'_'.$i; ?>_args" value="<?php echo _html_safe($entry['apply_args']); ?>"/>
<?php } } ?>
			<div class="icon"><?php echo $link; ?><img src="<?php echo $entry['icon']; ?>" alt=""/><?php echo $link_end; ?></div>
			<div class="thumbnail"><img src="<?php echo _html_safe_link($entry['thumbnail']); ?>" alt=""/></div>
			<div class="name"><?php echo $link.$entry['name'].$link_end; ?></div>
<?php if(isset($args['class'])) { $keys = array_keys($args['class']); foreach($keys as $k) { ?>
			<div class="<?php echo _html_safe($k); ?>"><?php echo $entry[$k]; ?></div>
<?php } } ?>
		</div>
