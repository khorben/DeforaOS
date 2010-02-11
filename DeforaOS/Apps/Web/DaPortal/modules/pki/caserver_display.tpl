<div class="pki ca">
	<h1 class="title caserver"><?php echo _html_safe($caserver['title']); ?></h1>
	<table>
		<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><?php echo _html_safe($caserver['country']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><?php echo _html_safe($caserver['state']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><?php echo _html_safe($caserver['locality']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><?php echo _html_safe($caserver['organization']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><?php echo _html_safe($caserver['section']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><?php echo _html_safe($caserver['cn']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><a href="mailto:<?php echo _html_safe($caserver['email']); ?>"><?php echo _html_safe($caserver['email']); ?></a></td></tr>
	</table>
	<div class="toolbar"><form action="<?php echo _html_link(); ?>" method="post"><input type="hidden" name="module" value="pki"/><input type="hidden" name="action" value="caserver_export"/><input type="hidden" name="id" value="<?php echo _html_safe($caserver['id']); ?>"/><?php echo _html_safe(KEY); ?>: <input type="password" name="key" value=""/> <input type="submit" value="<?php echo _html_safe(EXPORT); ?>" class="icon submit"/></form></div>
</div>
