<h1 class="title user"><?php echo _html_safe($title); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="<?php echo isset($user) ? 'update' : 'insert'; ?>"/>
<?php if(isset($user)) { ?>
	<input type="hidden" name="id" value="<?php echo _html_safe($user['user_id']); ?>"/>
<?php } ?>
	<table>
<?php if($admin) { ?>
		<tr><td class="field"><?php echo _html_safe(USERNAME); ?>:</td><td><input type="text" name="username" value="<?php if(isset($user)) echo _html_safe($user['username']); ?>"/></td>
<?php } else { ?>
		<tr><td class="field"><?php echo _html_safe(USERNAME); ?>:</td><td><?php if(isset($user)) echo _html_safe($user['username']); ?></td>
<?php } ?>
		<tr><td class="field"><?php echo _html_safe(PASSWORD); ?>:</td><td><input type="password" name="password1"/><br/><input type="password" name="password2"/></td></tr>
<?php if($admin) { ?>
		<tr><td class="field"><?php echo _html_safe(ENABLED); ?>:</td><td><input type="checkbox" name="enabled"<?php if(isset($user) && $user['enabled'] == SQL_TRUE) { ?> checked="checked"<?php } ?>/></td>
		<tr><td class="field"><?php echo _html_safe(ADMINISTRATOR); ?>:</td><td><input type="checkbox" name="admin"<?php if(isset($user['admin']) && $user['admin'] == SQL_TRUE) { ?> checked="checked"<?php } ?>/></td>
		<tr><td class="field">e-mail:</td><td><input type="text" name="email" value="<?php if(isset($user)) echo _html_safe($user['email']); ?>"/></td>
<?php } ?>
		<tr><td></td><td><a href="<?php echo _html_link('user', $admin ? 'admin' : FALSE); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="reset" class="icon reset"><?php echo _html_safe(RESET); ?></button> <button type="submit" class="icon submit"><?php echo _html_safe(isset($user) ? UPDATE : CREATE); ?></button></td></tr>
	</table>
</form>
