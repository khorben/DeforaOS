<h1><img src="modules/download/icon.png" alt=""/> <?php echo _html_safe(DOWNLOADS.': '.$file['name']); ?></h1>
<p>
<a href="index.php?module=download&amp;action=download&amp;id=<? echo _html_safe($file['id']); ?>"><img src="icons/16x16/save.png" alt-""/> Download file</a>
</p>
<table>
	<tr><td class="field">Name:</td><td><?php echo _html_safe($file['name']); ?></td></tr>
	<tr><td class="field">Owner:</td><td><?php echo _html_safe($file['user']); ?></td></tr>
	<tr><td class="field">Permissions:</td><td><?php echo _html_safe($file['mode']); ?></td></tr>
	<tr><td class="field">Creation time:</td><td><?php echo _html_safe($file['ctime']); ?></td></tr>
	<tr><td class="field">Modification time:</td><td><?php echo _html_safe($file['mtime']); ?></td></tr>
	<tr><td class="field">Access time:</td><td><?php echo _html_safe($file['atime']); ?></td></tr>
	<tr><td class="field">Size:</td><td><?php echo _html_safe($file['size']); ?></td></tr>
	<tr><td class="field">Comment:</td><td><?php echo _html_pre($file['content']); ?></td></tr>
</table>
