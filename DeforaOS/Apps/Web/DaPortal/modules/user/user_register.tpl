<form action="index.php" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="register"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(USERNAME); ?>:</td><td><input type="text" size="15" name="username"/></td></tr>
		<tr><td class="field">e-mail:</td><td><input type="text" size="30" name="email"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(REGISTER); ?>"/></td></tr>
	</table>
</form>
