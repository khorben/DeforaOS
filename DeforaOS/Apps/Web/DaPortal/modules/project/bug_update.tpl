<h1 class="title bug"><?php echo _html_safe($title); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_<?php echo isset($bug) ? 'update' : 'insert'; ?>"/>
<?php if(!isset($bug)) { ?>
	<input type="hidden" name="project_id" value="<?php echo _html_safe($project_id); ?>"/>
<?php } else { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($bug['id']); ?>"/>
	<input type="hidden" name="bug_id" value="<?php echo _html_safe($bug['bug_id']); ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(TITLE); ?>:</td><td colspan="3"><input type="text" name="title" value="<?php if(isset($bug['title'])) echo _html_safe($bug['title']); ?>" size="50"/></td></tr>
<?php if(isset($bug)) { ?>
		<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><select name="state">
<?php $states = _sql_enum('daportal_bug', 'state');
foreach($states as $s) { ?>
				<option value="<?php echo _html_safe($s); ?>"<?php if($bug['state'] == $s) { ?> selected="selected"<?php } ?>><?php echo _html_safe($s); ?></option>
<?php } ?>
			</select></td>
<?php } ?>
		<td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><select name="type">
<?php $types = _sql_enum('daportal_bug', 'type');
foreach($types as $t) { ?>
				<option value="<?php echo _html_safe($t); ?>"<?php if(isset($bug) && $bug['type'] == $t) { ?> selected="selected"<?php } ?>><?php echo _html_safe($t); ?></option>
<?php } ?>
			</select></td></tr>
		<tr><td class="field"><?php echo _html_safe(PRIORITY); ?>:</td><td><select name="priority">
<?php $priorities = _sql_enum('daportal_bug', 'priority');
foreach($priorities as $p) { ?>
				<option value="<?php echo _html_safe($p); ?>"<?php if(isset($bug) && $bug['priority'] == $p) { ?> selected="selected"<?php } ?>><?php echo _html_safe($p); ?></option>
<?php } ?>
			</select></td><?php if(isset($members)) { ?><td class="field"><?php echo _html_safe(ASSIGNED_TO); ?>:</td><td><select name="assigned_id">
			<option value=""<?php if(!isset($bug['assigned_id']) || !is_numeric($bug['assigned_id'])) { ?> selected="selected"<?php } ?>></option>
<?php foreach($members as $m){ ?>
			<option value="<?php echo _html_safe($m['id']); ?>"<?php if($bug['assigned_id'] == $m['id']) { ?> selected="selected"<?php } ?>><?php echo _html_safe($m['username']); ?></option>
<?php } ?>
			</select></td><?php } ?></tr>
		<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td colspan="3"><textarea name="content" cols="50" rows="10"><?php if(isset($bug['content'])) echo _html_safe($bug['content']); ?></textarea></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('project', 'bug_list', $project_id); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" value="<?php echo (!isset($bug)) ? _html_safe(SEND) : _html_safe(UPDATE); ?>" class="icon submit"/></td></tr>
	</table>
</form>
