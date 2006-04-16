<h1><img src="modules/project/bug.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_<? echo isset($bug) ? 'update' : 'insert'; ?>"/>
<? if(!isset($bug)) { ?>
	<input type="hidden" name="project_id" value="<? echo _html_safe($project_id); ?>"/>
<? } else { ?>
	<input type="hidden" name="bug_id" value="<? echo _html_safe($bug['id']); ?>"/>
<? } ?>
	<table>
		<tr><td class="field"><? echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<? echo _html_safe($bug['title']); ?>" size="50"/></td></tr>
		<tr><td class="field"><? echo _html_safe(DESCRIPTION); ?>:</td><td><textarea name="content" cols="50" rows="10"><? echo _html_safe($bug['content']); ?></textarea></td></tr>
<? if(isset($bug)) { ?>
		<tr><td class="field">State:</td><td><select name="state">
<? $states = array('New', 'Assigned', 'Closed', 'Fixed', 'Implemented');
foreach($states as $s) { ?>
				<option value="<? echo _html_safe($s); ?>"<? if($bug['state'] == $s) { ?> selected="selected"<? } ?>><? echo _html_safe($s); ?></option>
<? } ?>
			</select></td></tr>
<? } ?>
		<tr><td class="field"><? echo _html_safe(TYPE); ?>:</td><td><select name="type">
<? $types = array('Major', 'Minor', 'Functionality', 'Feature');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($bug['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field"><? echo _html_safe(PRIORITY); ?>:</td><td><select name="priority">
<? $priorities = array('Urgent', 'High', 'Medium', 'Low');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($bug['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="<? if(!isset($bug)) echo _html_safe(SEND); else echo _html_safe(UPDATE); ?>"/></td></tr>
	</table>
</form>
