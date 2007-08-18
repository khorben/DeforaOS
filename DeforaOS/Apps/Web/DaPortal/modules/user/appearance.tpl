<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="appearance"/>
	<table>
	<?php global $debug, $user_id; if(_user_admin($user_id)) { ?>
		<tr>
			<td class="field">Debug:</td>
			<td><input type="checkbox" name="debug"<?php if($debug == 1) { ?> checked="checked"<?php } ?>/></td>
		</tr>
	<?php } ?>
		<tr>
			<td class="field"><?php echo _html_safe(THEME); ?>:</td>
			<td><select name="theme">
	<?php foreach($themes as $t) { ?>
				<option<?php if($theme == $t) { ?> selected="selected"<?php } ?>><?php echo $t; ?></option>
	<?php } ?>
			</select></td>
		</tr>
		<tr>
			<td></td>
			<td><input type="submit" value="<?php echo _html_safe(SUBMIT); ?>"/></td>
		</tr>
	</table>
</form>
