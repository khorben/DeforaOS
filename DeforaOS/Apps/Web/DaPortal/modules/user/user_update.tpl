<h1><img src="modules/user/user.png" alt=""/> User modification</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="update"/>
	<table>
<? if($admin) { ?>
		<tr><td class="field">Username:</td><td><input type="text" name="username" value="<? echo _html_safe($user['username']); ?>"/></td>
<? } else { ?>
		<tr><td class="field">Username:</td><td><? echo _html_safe($user['username']); ?></td>
<? } ?>
<? if($admin || $super) { ?>
		<tr><td class="field">Password:</td><td><input type="password" name="password1"/><br/><input type="password" name="password2"/></td></tr>
<? } ?>
		<tr><td></td><td><input type="submit" value="Update"/></td></tr>
	</table>
</form>
