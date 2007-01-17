<form action="index.php" method="post">
	<input type="hidden" name="module" value="admin"/>
	<input type="hidden" name="action" value="settings_update"/>
	<table>
		<tr>
			<td class="field">Debugging:</td>
			<td><input type="checkbox" name="debug"<?php if($debug == 1) { ?> checked="checked"<?php } ?>/></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" value="Submit"/></td>
		</tr>
	</table>
</form>
