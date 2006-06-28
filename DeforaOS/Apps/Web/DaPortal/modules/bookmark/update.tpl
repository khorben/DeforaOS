<h1><img src="modules/bookmark/icon.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="bookmark"/>
	<input type="hidden" name="action" value="<?php echo isset($bookmark['id']) ? 'update' : 'insert'; ?>"/>
<?php if(isset($bookmark['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($bookmark['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(ADDRESS); ?>:</td><td><input type="text" name="url" value="<?php echo _html_safe($bookmark['url']); ?>" size="50"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<?php echo _html_safe($bookmark['title']); ?>" size="50"/></td></tr>
		<tr><td class="field">Description:</td><td><textarea name="content" cols="50" rows="3"><?php echo _html_safe($bookmark['content']); ?></textarea></td></tr>
		<tr><td class="field"><?php echo _html_safe(PUBLIC); ?>:</td><td><input type="checkbox" name="enabled"<?php if($bookmark['enabled'] == SQL_TRUE) { ?> checked="checked"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo isset($bookmark['id']) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
