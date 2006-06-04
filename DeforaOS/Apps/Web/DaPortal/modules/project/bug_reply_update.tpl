<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_reply<?php echo isset($reply['id']) ? '_update' : ''; ?>"/>
<?php if(!isset($reply['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo $bug['id']; ?>"/>
<?php } else { ?>
	<input type="hidden" name="id" value="<?php echo $reply['id']; ?>"/>
<?php } ?>
	<table>
		<tr><td class="field">Title:</td><td colspan="3"><input type="text" name="title" value="<?php echo _html_safe($reply['title']); ?>" size="50"/></td></tr>
<?php if($admin) { ?>
		<tr><td class="field">State:</td><td><select name="state">
				<option value=""<?php if($reply['state'] == '') { ?> selected="selected"<?php } ?>></option>
<?php $states = _sql_enum('daportal_bug', 'state');
foreach($states as $s) { ?>
				<option value="<?php echo _html_safe($s); ?>"<?php if($reply['state'] == $s) { ?> selected="selected"<?php } ?>><?php echo _html_safe($s); ?></option>
<?php } ?>
			</select></td>
		<td class="field">Type:</td><td><select name="type">
				<option value=""<?php if($reply['type'] == '') { ?> selected="selected"<?php } ?>></option>
<?php $types = _sql_enum('daportal_bug', 'type');
foreach($types as $t) { ?>
				<option value="<?php echo _html_safe($t); ?>"<?php if($reply['type'] == $t) { ?> selected="selected"<?php } ?>><?php echo _html_safe($t); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td class="field">Priority:</td><td><select name="priority">
				<option value=""<?php if($reply['priority'] == '') { ?> selected="selected"<?php } ?>></option>
<?php $priorities = _sql_enum('daportal_bug', 'priority');
foreach($priorities as $p) { ?>
				<option value="<?php echo _html_safe($p); ?>"<?php if($reply['priority'] == $p) { ?> selected="selected"<?php } ?>><?php echo _html_safe($p); ?></option>
<?php } ?>
			</select></td><td class="field"><?php echo _html_safe(ASSIGNED_TO); ?>:</td><td><?php echo _html_safe($reply['assigned']); ?></td></tr>
<?php } /* FIXME re-assign */ ?>
		<tr><td class="field">Description:</td><td colspan="3"><textarea name="content" cols="50" rows="10"><?php echo _html_safe($reply['content']); ?></textarea></td></tr>
		<tr><td></td><td><?php if(!isset($reply['id'])) { ?><input type="submit" name="preview" value="<?php echo _html_safe(PREVIEW); ?>"/> <input type="submit" name="submit" value="<?php echo _html_safe(SEND); ?>"/><?php } else { ?><input type="submit" value="<?php echo _html_safe(UPDATE); ?>"/><?php } ?></td></tr>
	</table>
</form>
