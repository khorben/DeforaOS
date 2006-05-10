<form method="get" action="index.php">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_list"/>
	<table>
		<tr><td class="field"><? echo _html_safe(PROJECT_NAME); ?>:</td><td><input type="text" name="project" value="<? echo _html_safe(stripslashes($args['project'])); ?>" size="20"/></td>
		<td class="field"><? echo _html_safe(SUBMITTER); ?>:</td><td><input type="text" name="username" value="<? echo _html_safe(stripslashes($args['username'])); ?>" size="20"/></td></tr>
		<tr><td class="field"><? echo _html_safe(STATE); ?>:</td><td><select name="state">
				<option value=""<? if($args['state'] == '') { ?> selected="selected"<? } ?>>All</option>
<? $states = _sql_enum('daportal_bug', 'state');
foreach($states as $s) { ?>
				<option value="<? echo _html_safe($s); ?>"<? if($args['state'] == $s) { ?> selected="selected"<? } ?>><? echo _html_safe($s); ?></option>
<? } ?>
			</select></td>
		<td class="field"><? echo _html_safe(TYPE); ?>:</td><td><select name="type">
				<option value=""<? if($args['type'] == '') { ?> selected="selected"<? } ?>>All</option>
<? $types = _sql_enum('daportal_bug', 'type');
foreach($types as $t) { ?>
				<option value="<? echo _html_safe($t); ?>"<? if($args['type'] == $t) { ?> selected="selected"<? } ?>><? echo _html_safe($t); ?></option>
<? } ?>
			</select></td></tr>
		<tr><td class="field"><? echo _html_safe(PRIORITY); ?>:</td><td><select name="priority">
				<option value=""<? if($args['priority'] == '') { ?> selected="selected"<? } ?>>All</option>
<? $priorities = _sql_enum('daportal_bug', 'priority');
foreach($priorities as $p) { ?>
				<option value="<? echo _html_safe($p); ?>"<? if($args['priority'] == $p) { ?> selected="selected"<? } ?>><? echo _html_safe($p); ?></option>
<? } ?>
			</select></td>
		<td></td><td><input type="submit" value="<? echo _html_safe(FILTER); ?>"/></td></tr>
	</table>
</form>
