<h1><img src="modules/probe/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="probe"/>
	<input type="hidden" name="action" value="<? echo isset($host) ? 'host_update' : 'host_insert'; ?>"/>
	<table>
		<tr><td class="field">Hostname&nbsp;:</td><td><input type="text" name="hostname" value="<? echo _html_safe($host['hostname']); ?>"/></td></tr>
		<tr><td class="field">Comment&nbsp;:</td><td><textarea name="comment"><? echo _html_safe($host['comment']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="<? echo isset($host) ? 'Update' : 'Create'; ?>"/></td></tr>
	</table>
</form>
