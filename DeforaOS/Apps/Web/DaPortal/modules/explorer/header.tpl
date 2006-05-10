	<div id="explorer_<?php echo $explorer_id; ?>" class="listing_<?php echo _html_safe($view); ?>">
		<div class="header">
			<div class="icon"></div>
			<div class="name"><?php if($args['sort']) _explorer_sort($args['module'], $args['action'], $args['args'], 'name', $args['sort'], NAME); else { echo _html_safe(NAME); } ?></div>
<?php if(isset($args['class'])) { $keys = array_keys($args['class']); foreach($keys as $k) { ?>
			<div class="<?php echo _html_safe($k); ?>"><?php if($args['sort']) _explorer_sort($args['module'], $args['action'], $args['args'], $k, $args['sort'], $args['class'][$k]); else { echo _html_safe($args['class'][$k]); } ?></div>
<?php } } ?>
		</div>
