<h1><img src="modules/webmail/icon.png" alt=""/> Webmail</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="webmail"/>
	<input type="hidden" name="action" value="login"/>
	<table>
<? if($message) { ?>
	<tr><td></td><td><strong><? echo $message; ?></strong></td></tr>
<? } ?>
	<tr><td class="field"><? echo _html_safe(USERNAME); ?>:</td><td><input type="text" name="username"<? if($username) print(' value="'._html_safe($username).'"'); ?>/></td></tr>
	<tr><td class="field"><? echo _html_safe(PASSWORD); ?>:</td><td><input type="password" name="password"/></td></tr>
	<tr><td></td><td><input type="submit" name="submit" value="<? echo _html_safe(LOGIN); ?>"/></td></tr>
	</table>
</form>
