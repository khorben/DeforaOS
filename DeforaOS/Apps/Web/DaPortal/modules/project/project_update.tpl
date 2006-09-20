<h1><img src="modules/project/icon.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="<?php echo isset($project) ? 'update' : 'insert'; ?>"/>
<?php if(isset($project['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($project['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="name" value="<?php if(isset($project['name'])) echo _html_safe($project['name']); ?>" size="20"/></td></tr>
		<tr><td class="field">Short description:</td><td><input type="text" name="title" value="<?php if(isset($project['title'])) echo _html_safe($project['title']); ?>" size="50"/></td></tr>
		<tr><td class="field">Long description:</td><td><textarea name="content" cols="50" rows="10"><?php if(isset($project['content'])) echo _html_safe($project['content']); ?></textarea></td></tr>
		<tr><td class="field">CVS path:</td><td><input type="text" name="cvsroot" value="<?php if(isset($project['cvsroot'])) echo _html_safe($project['cvsroot']); ?>" size="50"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo isset($project) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
