<form action="index.php" method="get">
	<input type="hidden" name="module" value="wiki"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(LOOK_FOR_A_PAGE); ?>:</td><td><input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(SEARCH); ?>" class="icon search"/></td></tr>
	</table>
</form>
