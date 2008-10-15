<form action="index.php" method="post" enctype="multipart/form-data">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="download_insert"/>
	<input type="hidden" name="id" value="<?php echo _html_safe($project['id']); ?>"/>
	<table>
		<tr><td class="field">File:</td><td><input type="file" name="file" value=""/></td></tr>
		<tr><td class="field">Directory:</td><td><input type="text" name="directory" value="<?php if(isset($directory)) echo _html_safe($directory); ?>"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(UPLOAD); ?>"/></td></tr>
	</table>
</form>
