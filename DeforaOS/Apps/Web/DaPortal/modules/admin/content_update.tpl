<h1><img src="modules/admin/edit.png" alt=""/> Content update</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="admin"/>
	<input type="hidden" name="action" value="content_update"/>
	<input type="hidden" name="id" value="<? echo $id; ?>"/>
	<table>
		<tr><td class="field">Title:</td><td><input type="text" name="title" value="<? echo _html_safe($content['title']); ?>" size="80"/></td></tr>
		<tr><td class="field">Date:</td><td><input type="text" name="timestamp" value="<? echo _html_safe($content['timestamp']); ?>" size="40"/></td></tr>
		<tr><td class="field">Content:</td><td><textarea name="content" rows="20" cols="80"><? echo _html_safe($content['content']); ?></textarea></td></tr>
		<tr><td class="field">Enabled:</td><td><select name="enabled">
				<option value="0"<? if($content['enabled'] == 'f') echo ' selected="selected"'; ?>>No</option>
				<option value="1"<? if($content['enabled'] == 't') echo ' selected="selected"'; ?>>Yes</option>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="Update"/></td></tr>
	</table>
</form>
