<div class="pki ca">
	<h1 class="title pki"><?php echo _html_safe($ca['title']); ?></h1>
	<table>
		<tr><td class="field">Country:</td><td><?php echo _html_safe($ca['country']); ?></td></tr>
		<tr><td class="field">State:</td><td><?php echo _html_safe($ca['state']); ?></td></tr>
		<tr><td class="field">Locality:</td><td><?php echo _html_safe($ca['locality']); ?></td></tr>
		<tr><td class="field">Organization:</td><td><?php echo _html_safe($ca['organization']); ?></td></tr>
		<tr><td class="field">Section (OU):</td><td><?php echo _html_safe($ca['section']); ?></td></tr>
		<tr><td class="field">Common Name (CN):</td><td><?php echo _html_safe($ca['cn']); ?></td></tr>
		<tr><td class="field">e-mail:</td><td><?php echo _html_safe($ca['email']); ?></td></tr>
	</table>
	<div class="toolbar"><a href="<?php echo _html_link('pki', 'ca_import', $ca['id']); ?>"><button>Import</button></a><span class="middot"> &middot; </span><a href="<?php echo _html_link('pki', 'ca_export', $ca['id']); ?>"><button>Export</button></a></div>
</div>
