	<div id="explorer_<?php echo $explorer_id; ?>" class="listing_<?php echo _html_safe($view); ?>">
<?php if(!isset($args['header']) || $args['header'] != 0) { ?>
		<div class="header">
			<span class="icon"></span>
			<span class="name<?php if(isset($args['sort'])) _explorer_sort($args['module'], $args['action'], isset($args['args']) ? $args['args'] : '', 'name', $args['sort'], NAME); else { echo '">'._html_safe(NAME); } ?></span>
<?php foreach($class as $c) { ?>
			<span class="<?php echo _html_safe($c); ?><?php if(isset($args['sort'])) _explorer_sort($args['module'], $args['action'], isset($args['args']) ? $args['args'] : '', $c, $args['sort'], $args['class'][$c]); else { echo '">'._html_safe($args['class'][$c]); } ?></span>
<?php } ?>
		</div>
<?php } ?>
