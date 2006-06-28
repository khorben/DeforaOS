<h1><img src="modules/content/icon.png" alt=""/> Content update</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="content"/>
	<input type="hidden" name="action" value="update"/>
	<input type="hidden" name="id" value="<?php echo $id; ?>"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<?php echo _html_safe($content['title']); ?>" size="50"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(DATE); ?>:</td><td><input type="text" name="timestamp" value="<?php echo _html_safe($content['timestamp']); ?>" size="20"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(CONTENT); ?>:</td><td><textarea name="content" cols="50" rows="10"><?php echo _html_safe($content['content']); ?></textarea></td></tr>
		<tr><td class="field">Enabled:</td><td><select name="enabled">
				<option value="0"<?php if($content['enabled'] == SQL_FALSE) echo ' selected="selected"'; ?>>No</option>
				<option value="1"<?php if($content['enabled'] == SQL_TRUE) echo ' selected="selected"'; ?>>Yes</option>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(UPDATE); ?>"/></td></tr>
	</table>
</form>
