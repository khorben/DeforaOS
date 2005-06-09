<form method="get" action="index.php">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_list"/>
	<table>
		<tr><td class="name">Project name:</td><td><input type="text" name="project" value="<? echo _html_safe(stripslashes($args['project'])); ?>"/></td></tr>
		<tr><td class="name">State:</td><td><select name="state">
<? $states = array('', 'New', 'Assigned', 'Closed', 'Fixed', 'Implemented');
foreach($states as $s) { ?>
				<option value="<? echo _html_safe($s); ?>"<? if($args['state'] == $s) { ?> selected="selected"<? } ?>><? echo _html_safe($s); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="name">Type:</td><td><select name="type">
<? $types = array('', 'Major', 'Minor', 'Functionality', 'Feature');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($args['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="name">Priorité:</td><td><select name="priority">
<? $priorities = array('', 'Urgent', 'High', 'Medium', 'Low');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($args['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td></td><td><input type="submit" value="Filter"/></td></tr>
	</table>
</form>
