<? _error('User registration is currently disabled'); ?>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="register"/>
	<table>
		<tr><td class="field">Username:</td><td><input type="text" size="15" name="username"/></td></tr>
		<tr><td class="field">e-mail:</td><td><input type="text" size="30" name="email"/></td></tr>
		<tr><td></td><td><input type="submit" value="Register"/></td></tr>
	</table>
</form>
