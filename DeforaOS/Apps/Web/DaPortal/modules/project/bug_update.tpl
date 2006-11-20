<h1 class="title bug"><?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_<?php echo isset($bug) ? 'update' : 'insert'; ?>"/>
<?php if(!isset($bug)) { ?>
	<input type="hidden" name="project_id" value="<?php echo _html_safe($project_id); ?>"/>
<?php } else { ?>
	<input type="hidden" name="bug_id" value="<?php echo _html_safe($bug['id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<?php if(isset($bug['title'])) echo _html_safe($bug['title']); ?>" size="50"/></td></tr>
		<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td><textarea name="content" cols="50" rows="10"><?php if(isset($bug['content'])) echo _html_safe($bug['content']); ?></textarea></td></tr>
<?php if(isset($bug)) { ?>
		<tr><td class="field">State:</td><td><select name="state">
<?php $states = _sql_enum('daportal_bug', 'state');
foreach($states as $s) { ?>
				<option value="<?php echo _html_safe($s); ?>"<?php if($bug['state'] == $s) { ?> selected="selected"<?php } ?>><?php echo _html_safe($s); ?></option>
<?php } ?>
			</select></td></tr>
<?php } ?>
		<tr><td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><select name="type">
<?php $types = _sql_enum('daportal_bug', 'type');
foreach($types as $t) { ?>
				<option value="<?php echo _html_safe($t); ?>"<?php if($bug['type'] == $t) { ?> selected="selected"<?php } ?>><?php echo _html_safe($t); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td class="field"><?php echo _html_safe(PRIORITY); ?>:</td><td><select name="priority">
<?php $priorities = _sql_enum('daportal_bug', 'priority');
foreach($priorities as $p) { ?>
				<option value="<?php echo _html_safe($p); ?>"<?php if($bug['priority'] == $p) { ?> selected="selected"<?php } ?>><?php echo _html_safe($p); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="<?php if(!isset($bug)) echo _html_safe(SEND); else echo _html_safe(UPDATE); ?>"/></td></tr>
	</table>
</form>
