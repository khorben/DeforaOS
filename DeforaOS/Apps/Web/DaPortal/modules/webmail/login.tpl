<h1 class="title webmail">Webmail</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="webmail"/>
	<input type="hidden" name="action" value="login"/>
	<table>
<?php if(isset($message)) { ?>
	<tr><td></td><td><strong><?php echo _html_safe($message); ?></strong></td></tr>
<?php } ?>
	<tr><td class="field"><?php echo _html_safe(USERNAME); ?>:</td><td><input type="text" name="username"<?php if(isset($username)) echo ' value="'._html_safe($username).'"'; ?>/></td></tr>
	<tr><td class="field"><?php echo _html_safe(PASSWORD); ?>:</td><td><input type="password" name="password"/></td></tr>
	<tr><td></td><td><input type="submit" name="submit" value="<?php echo _html_safe(LOGIN); ?>"/></td></tr>
	</table>
</form>
