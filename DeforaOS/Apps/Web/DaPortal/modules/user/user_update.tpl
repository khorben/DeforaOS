<h1><img src="modules/user/user.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="<? echo isset($user) ? 'update' : 'insert'; ?>"/>
<? if(isset($user)) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($user['user_id']); ?>"/>
<? } ?>
	<table>
<? if($admin) { ?>
		<tr><td class="field">Username:</td><td><input type="text" name="username" value="<? echo _html_safe($user['username']); ?>"/></td>
<? } else { ?>
		<tr><td class="field">Username:</td><td><? echo _html_safe($user['username']); ?></td>
<? } ?>
		<tr><td class="field">Password:</td><td><input type="password" name="password1"/><br/><input type="password" name="password2"/></td></tr>
<? if($admin) { ?>
		<tr><td class="field">Enabled:</td><td><input type="checkbox" name="enabled"<? if($user['enabled'] == 't') { ?> checked="checked"<? } ?>/></td>
		<tr><td class="field">Administrator:</td><td><input type="checkbox" name="admin"<? if($user['admin'] == 't') { ?> checked="checked"<? } ?>/></td>
		<tr><td class="field">e-mail:</td><td><input type="text" name="email" value="<? echo _html_safe($user['email']); ?>"/></td>
<? } ?>
		<tr><td></td><td><input type="submit" value="<? echo isset($user) ? 'Update' : 'Create'; ?>"/></td></tr>
	</table>
</form>
