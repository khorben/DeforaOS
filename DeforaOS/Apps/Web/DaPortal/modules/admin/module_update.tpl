<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="admin"/>
	<input type="hidden" name="action" value="module_insert"/>
	<table>
		<tr>
			<td class="field"><?php echo _html_safe(NAME); ?>:</td>
			<td><input type="text" name="name"/></td>
		</tr>
		<tr>
			<td></td>
			<td><a href="<?php echo _html_link('admin', 'admin'); ?>"><button class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" value="<?php echo _html_safe(INSERT); ?>" class="icon submit"/></td>
		</tr>
	</table>
</form>
