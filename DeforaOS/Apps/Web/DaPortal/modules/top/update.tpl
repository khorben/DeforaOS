<h1 class="top"><?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="top"/>
	<input type="hidden" name="action" value="<?php echo _html_safe($action); ?>"/>
<?php if(isset($top)) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($top['top_id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field">Name:</td><td><input type="text" name="name" value="<?php echo _html_safe($top['name']); ?>" size="40"/></td></tr>
		<tr><td class="field">Address:</td><td><input type="text" name="link" value="<?php echo _html_safe($top['link']); ?>" size="40"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo isset($top) ? 'Update' : 'Create'; ?>"/>
	</table>
</form>
