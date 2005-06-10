<h1><img src="modules/project/bug.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_<? echo isset($bug) ? 'update' : 'insert'; ?>"/>
	<input type="hidden" name="project_id" value="<? echo _html_safe($project_id); ?>"/>
	<table>
		<tr><td class="field">Title:</td><td><input type="text" name="title" value="<? echo _html_safe($bug['title']); ?>" size="80"/></td></tr>
		<tr><td class="field">Description:</td><td><textarea name="content" cols="80" rows="10"><? echo _html_safe($bug['content']); ?></textarea></td></tr>
		<tr><td class="field">Type:</td><td><select name="type">
<? $types = array('Major', 'Minor', 'Functionality', 'Feature');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($args['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field">Priority:</td><td><select name="priority">
<? $priorities = array('Urgent', 'High', 'Medium', 'Low');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($args['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="Submit"/></td></tr>
	</table>
</form>
