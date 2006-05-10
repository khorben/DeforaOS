<h1><img src="modules/user/user.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="<?php echo isset($user) ? 'update' : 'insert'; ?>"/>
<?php if(isset($user)) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($user['user_id']); ?>"/>
<?php } ?>
	<table>
<?php if($admin) { ?>
		<tr><td class="field">Username:</td><td><input type="text" name="username" value="<?php echo _html_safe($user['username']); ?>"/></td>
<?php } else { ?>
		<tr><td class="field">Username:</td><td><?php echo _html_safe($user['username']); ?></td>
<?php } ?>
		<tr><td class="field">Password:</td><td><input type="password" name="password1"/><br/><input type="password" name="password2"/></td></tr>
<?php if($admin) { ?>
		<tr><td class="field">Enabled:</td><td><input type="checkbox" name="enabled"<?php if($user['enabled'] == 't') { ?> checked="checked"<?php } ?>/></td>
		<tr><td class="field">Administrator:</td><td><input type="checkbox" name="admin"<?php if($user['admin'] == 't') { ?> checked="checked"<?php } ?>/></td>
		<tr><td class="field">e-mail:</td><td><input type="text" name="email" value="<?php echo _html_safe($user['email']); ?>"/></td>
<?php } ?>
		<tr><td></td><td><input type="submit" value="<?php echo isset($user) ? 'Update' : 'Create'; ?>"/></td></tr>
	</table>
</form>
