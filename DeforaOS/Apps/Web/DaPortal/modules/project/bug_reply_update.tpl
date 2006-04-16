<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_reply_<? echo isset($reply['id']) ? 'update' : 'insert'; ?>"/>
<? if(!isset($reply['id'])) { ?>
	<input type="hidden" name="id" value="<? echo $bug['id']; ?>"/>
<? } else { ?>
	<input type="hidden" name="id" value="<? echo $reply['id']; ?>"/>
<? } ?>
	<hr/>
	<table>
		<tr><td class="field">Title:</td><td><input type="text" name="title" value="<? echo _html_safe($reply['title']); ?>" size="50"/></td></tr>
		<tr><td class="field">Description:</td><td><textarea name="content" cols="50" rows="10"><? echo _html_safe($reply['content']); ?></textarea></td></tr>
<? if($admin) { ?>
		<tr><td class="field">State:</td><td><select name="state">
<? $states = array('New', 'Assigned', 'Closed', 'Fixed', 'Implemented');
foreach($states as $s) { ?>
				<option value="<? echo _html_safe($s); ?>"<? if($bug['state'] == $s) { ?> selected="selected"<? } ?>><? echo _html_safe($s); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field">Type:</td><td><select name="type">
<? $types = array('Major', 'Minor', 'Functionality', 'Feature');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($bug['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field">Priority:</td><td><select name="priority">
<? $priorities = array('Urgent', 'High', 'Medium', 'Low');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($bug['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td></tr>
<? } /* FIXME re-assign */ ?>
		<tr><td></td><td><input type="submit" value="<? echo _html_safe(isset($reply['id']) ? UPDATE : SEND); ?>"/></td></tr>
	</table>
</form>
