<div class="pki ca">
	<h1 class="title caclient"><?php echo _html_safe($caclient['title']); ?></h1>
	<table>
		<tr><td class="field">Country:</td><td><?php echo _html_safe($caclient['country']); ?></td></tr>
		<tr><td class="field">State:</td><td><?php echo _html_safe($caclient['state']); ?></td></tr>
		<tr><td class="field">Locality:</td><td><?php echo _html_safe($caclient['locality']); ?></td></tr>
		<tr><td class="field">Organization:</td><td><?php echo _html_safe($caclient['organization']); ?></td></tr>
		<tr><td class="field">Section (OU):</td><td><?php echo _html_safe($caclient['section']); ?></td></tr>
		<tr><td class="field">Common Name (CN):</td><td><?php echo _html_safe($caclient['cn']); ?></td></tr>
		<tr><td class="field">e-mail:</td><td><?php echo _html_safe($caclient['email']); ?></td></tr>
	</table>
	<div class="toolbar"><form action="index.php" method="post"><input type="hidden" name="module" value="pki"/><input type="hidden" name="action" value="caclient_export"/><input type="hidden" name="id" value="<?php echo _html_safe($caclient['id']); ?>"/><input type="submit" value="Export"/></form></div>
</div>
