<div class="pki ca">
	<h1 class="title pki"><?php echo _html_safe($ca['title']); ?></h1>
	<table>
		<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><?php echo _html_safe($ca['country']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><?php echo _html_safe($ca['state']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><?php echo _html_safe($ca['locality']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><?php echo _html_safe($ca['organization']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><?php echo _html_safe($ca['section']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><?php echo _html_safe($ca['cn']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><a href="mailto:<?php echo _html_safe($ca['email']); ?>"><?php echo _html_safe($ca['email']); ?></a></td></tr>
	</table>
	<div class="toolbar"><a href="<?php echo _html_link('pki', 'ca_import', $ca['id']); ?>"><button><?php echo _html_safe(IMPORT); ?></button></a><span class="middot"> &middot; </span><a href="<?php echo _html_link('pki', 'ca_export', $ca['id']); ?>"><button><?php echo _html_safe(EXPORT); ?></button></a></div>
</div>
