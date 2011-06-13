<h1 class="title project"><?php echo _html_safe($title); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="<?php echo isset($project) ? 'update' : 'insert'; ?>"/>
<?php if(isset($project['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($project['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<?php if(isset($project['name'])) echo _html_safe($project['name']); ?>" size="20"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(SYNOPSIS); ?>:</td><td><input type="text" name="synopsis" value="<?php if(isset($project['synopsis'])) echo _html_safe($project['synopsis']); ?>" size="50"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td><textarea name="content" cols="50" rows="10"><?php if(isset($project['content'])) echo _html_safe($project['content']); ?></textarea></td></tr>
		<tr><td class="field"><?php echo _html_safe(CVS_PATH); ?>:</td><td><input type="text" name="cvsroot" value="<?php if(isset($project['cvsroot'])) echo _html_safe($project['cvsroot']); ?>" size="50"/></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('project', isset($project['id']) ? FALSE : 'list', isset($project['id']) ? $project['id'] : FALSE); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="submit" class="icon submit"><?php echo isset($project) ? _html_safe(UPDATE) : _html_safe(SEND); ?></button></td></tr>
	</table>
</form>
