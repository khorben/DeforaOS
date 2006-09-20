<h1><img src="modules/category/icon.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="category"/>
	<input type="hidden" name="action" value="<?php echo isset($category) ? 'update' : 'insert'; ?>"/>
<?php if(isset($category)) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($category['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<?php if(isset($category['name'])) echo _html_safe($category['name']); ?>" size="50"/></td></tr>
		<tr><td class="field">Description:</td><td><input type="text" name="content" value="<?php if(isset($category['content'])) echo _html_safe($category['content']); ?>" size="50"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo isset($category) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
