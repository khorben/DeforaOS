<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_reply_<? echo isset($reply['id']) ? 'update' : 'insert'; ?>"/>
<? if(!isset($reply['id'])) { ?>
	<input type="hidden" name="id" value="<? echo $bug['id']; ?>"/>
<? } else { ?>
	<input type="hidden" name="id" value="<? echo $reply['id']; ?>"/>
<? } ?>
	<table>
		<tr><td class="field">Title:</td><td colspan="3"><input type="text" name="title" value="<? echo _html_safe($reply['title']); ?>" size="50"/></td></tr>
<? if($admin) { ?>
		<tr><td class="field">State:</td><td><select name="state">
				<option value=""<? if($reply['state'] == '') { ?> selected="selected"<? } ?>></option>
<? $states = _sql_enum('daportal_bug', 'state');
foreach($states as $s) { ?>
				<option value="<? echo _html_safe($s); ?>"<? if($reply['state'] == $s) { ?> selected="selected"<? } ?>><? echo _html_safe($s); ?></option>
<? } ?>
			</select></td>
		<td class="field">Type:</td><td><select name="type">
				<option value=""<? if($reply['type'] == '') { ?> selected="selected"<? } ?>></option>
<? $types = _sql_enum('daportal_bug', 'type');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($reply['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field">Priority:</td><td><select name="priority">
				<option value=""<? if($reply['priority'] == '') { ?> selected="selected"<? } ?>></option>
<? $priorities = _sql_enum('daportal_bug', 'priority');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($reply['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td><td class="field"><? echo _html_safe(ASSIGNED_TO); ?>:</td><td><? echo _html_safe($reply['assigned']); ?></td></tr>
<? } /* FIXME re-assign */ ?>
		<tr><td class="field">Description:</td><td colspan="3"><textarea name="content" cols="50" rows="10"><? echo _html_safe($reply['content']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="<? echo _html_safe(isset($reply['id']) ? UPDATE : SEND); ?>"/></td></tr>
	</table>
</form>
