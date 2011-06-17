<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_reply<?php echo isset($reply['id']) ? '_update' : ''; ?>"/>
<?php if(!isset($reply['id'])) { ?>
	<input type="hidden" name="id" value="<?php echo $bug['id']; ?>"/>
	<input type="hidden" name="bug_id" value="<?php echo $bug['bug_id']; ?>"/>
<?php } else { ?>
	<input type="hidden" name="id" value="<?php echo $reply['id']; ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td colspan="3"><input type="text" name="title" value="<?php echo _html_safe($reply['title']); ?>" size="50"/></td></tr>
<?php if($admin || $member) { ?>
		<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><select name="state">
				<option value=""<?php if(!isset($reply['state'])) { ?> selected="selected"<?php } ?>></option>
<?php $states = _sql_enum('daportal_bug_reply', 'state');
foreach($states as $s) { ?>
				<option value="<?php echo _html_safe($s); ?>"<?php if(isset($reply['state']) && $reply['state'] == $s) { ?> selected="selected"<?php } ?>><?php echo _html_safe($s); ?></option>
<?php } ?>
			</select></td>
		<td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><select name="type">
				<option value=""<?php if(!isset($reply['type'])) { ?> selected="selected"<?php } ?>></option>
<?php $types = _sql_enum('daportal_bug_reply', 'type');
foreach($types as $t) { ?>
				<option value="<?php echo _html_safe($t); ?>"<?php if(isset($reply['type']) && $reply['type'] == $t) { ?> selected="selected"<?php } ?>><?php echo _html_safe($t); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td class="field"><?php echo _html_safe(PRIORITY); ?>:</td><td><select name="priority">
				<option value=""<?php if(!isset($reply['priority'])) { ?> selected="selected"<?php } ?>></option>
<?php $priorities = _sql_enum('daportal_bug_reply', 'priority');
foreach($priorities as $p) { ?>
				<option value="<?php echo _html_safe($p); ?>"<?php if(isset($reply['priority']) && $reply['priority'] == $p) { ?> selected="selected"<?php } ?>><?php echo _html_safe($p); ?></option>
<?php } ?>
			</select></td><td class="field"><?php echo _html_safe(ASSIGNED_TO); ?>:</td><td><select name="assigned_id">
				<option value=""<?php if(!isset($reply['assigned_id']) || !is_numeric($reply['assigned_id']) == '') { ?> selected="selected"<?php } ?>></option>
<?php foreach($members as $m) { ?>
				<option value="<?php echo _html_safe($m['id']); ?>"<?php if(isset($reply['assigned_id']) && $reply['assigned_id'] == $m['id']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($m['username']); ?></option>
<?php } ?>
			</select></td></tr>
<?php } ?>
		<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td colspan="3"><textarea name="content" cols="50" rows="10"><?php if(isset($reply['content'])) echo _html_safe($reply['content']); ?></textarea></td></tr>
		<tr><td></td><td><?php if(!isset($reply['id'])) { ?><button type="submit" name="preview" class="icon preview"><?php echo _html_safe(PREVIEW); ?></button> <button type="submit" name="submit" class="icon submit"><?php echo _html_safe(SEND); ?></button><?php } else { ?><button type="submit" class="icon submit"><?php echo _html_safe(UPDATE); ?></button><?php } ?></td></tr>
	</table>
</form>
