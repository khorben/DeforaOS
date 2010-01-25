<h1 class="title user"><?php echo _html_safe(USER_LOGIN); ?></h1>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="user"/>
	<input type="hidden" name="action" value="login"/>
	<table>
		<tr><td class="field"><?php echo _html_safe(USERNAME); ?>:</td><td><input type="text" name="username"<?php if($username) print(' value="'._html_safe($username).'"'); ?>/></td></tr>
		<tr><td class="field"><?php echo _html_safe(PASSWORD); ?>:</td><td><input type="password" name="password"/></td></tr>
	<tr><td></td><td><input type="submit" name="submit" value="<?php echo _html_safe(LOGIN); ?>"/></td></tr>
<?php if($register) { ?>
	<tr><td></td><td><a href="<?php echo _html_link('user', 'register'); ?>"><?php echo _html_safe(REGISTER); ?>...</a></td></tr>
<?php } ?>
	</table>
</form>
