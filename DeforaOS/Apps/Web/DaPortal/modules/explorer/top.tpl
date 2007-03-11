<script type="text/javascript" src="js/explorer.js"></script>
<form name="explorer_<?php echo $explorer_id; ?>" class="explorer" action="index.php" method="post">
	<div>
		<input type="hidden" name="module" value="explorer"/>
		<input type="hidden" name="action" value="apply"/>
		<input type="hidden" name="apply" value=""/>
<?php if(strlen($module) && strlen($action)) { ?>
		<input type="hidden" name="link_module" value="<?php echo _html_safe_link($module); ?>"/>
		<input type="hidden" name="link_action" value="<?php echo _html_safe_link($action); ?>"/>
<?php if(strlen($id)) { ?>
		<input type="hidden" name="link_id" value="<?php echo _html_safe_link($id); ?>"/>
<?php } } ?>
