<form action="<?php echo _html_link(); ?>" method="post" enctype="multipart/form-data">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="screenshot_insert"/>
	<input type="hidden" name="id" value="<?php echo _html_safe($project['id']); ?>"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(FILE); ?>:</td><td><input type="file" name="file" value=""/></td></tr>
		<tr><td class="field"><?php echo _html_safe(DIRECTORY); ?>:</td><td><input type="text" name="directory" value="<?php if(isset($directory)) echo _html_safe($directory); ?>"/></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('project', 'download', $project['id']); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="submit" class="icon submit"><?php echo _html_safe(UPLOAD); ?></button></td></tr>
	</table>
</form>
