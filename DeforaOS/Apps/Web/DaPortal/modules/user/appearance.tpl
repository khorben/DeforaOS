<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="appearance"/>
	<table>
<?php global $debug, $user_id;
require_once('./system/user.php'); if(_user_admin($user_id)) { ?>
		<tr>
			<td class="field">Debug:</td>
			<td><input type="checkbox" name="debug"<?php if($debug == 1) { ?> checked="checked"<?php } ?>/></td>
		</tr>
	<?php } ?>
		<tr>
			<td class="field"><?php echo _html_safe(DEFAULT_THEME); ?>:</td>
			<td><select name="theme">
	<?php foreach($themes as $t) { ?>
				<option<?php if($theme == $t) { ?> selected="selected"<?php } ?>><?php echo _html_safe($t); ?></option>
	<?php } ?>
			</select></td>
		</tr>
		<tr>
			<td class="field"><?php echo _html_safe(DEFAULT_VIEW); ?>:</td>
			<td><select name="view">
				<option><?php echo _html_safe(NONE); ?></option>
	<?php $keys = array_keys($views); foreach($keys as $k) { ?>
				<option value="<?php echo _html_safe($k); ?>"<?php if(isset($view) && $view == $k) { ?> selected="selected"<?php } ?>><?php echo _html_safe($views[$k]); ?></option>
	<?php } ?>
			</select></td>
		</tr>
		<tr>
			<td></td>
			<td><a href="<?php echo _html_link('user'); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="reset" class="icon reset"><?php echo _html_safe(RESET); ?></button> <button type="submit" class="icon submit"><?php echo _html_safe(SUBMIT); ?></button></td>
		</tr>
	</table>
</form>
