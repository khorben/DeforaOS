<h1><img src="modules/download/icon.png" alt=""/> <?php echo _html_safe(DOWNLOADS.': '.$file['name']); ?></h1>
<p>
<a href="index.php?module=download&amp;action=download&amp;id=<?php echo _html_safe($file['id']); ?>"><img src="icons/16x16/save.png" alt-""/> Download file</a>
</p>
<table>
	<tr><td class="field">Name:</td><td><?php echo _html_safe($file['name']); ?></td></tr>
	<tr><td class="field">Type:</td><td><?php echo _html_safe($file['mime']); ?></td></tr>
	<tr><td class="field">Owner:</td><td><a href="index.php?module=user&amp;id=<?php echo _html_safe($file['user_id']); ?>"><?php echo _html_safe($file['user']); ?></a></td></tr>
	<tr><td class="field">Permissions:</td><td style="font-family: monospace"><?php echo _html_safe($file['mode']); ?></td></tr>
	<tr><td class="field">Creation time:</td><td><?php echo _html_safe($file['ctime']); ?></td></tr>
	<tr><td class="field">Modification time:</td><td><?php echo _html_safe($file['mtime']); ?></td></tr>
	<tr><td class="field">Access time:</td><td><?php echo _html_safe($file['atime']); ?></td></tr>
	<tr><td class="field">Size:</td><td><?php echo _html_safe($file['size']); ?></td></tr>
	<tr><td class="field">Comment:</td><td><?php echo _html_pre($file['content']); ?></td></tr>
</table>
<?php if(strncmp('image/', $file['mime'], 6) == 0) { ?>
<h2>Image preview</h2>
<img src="index.php?module=download&amp;action=download&amp;id=<?php echo _html_safe($file['id']); ?>" alt="" style="max-height: 100px; max-width: 100px"/>
<?php } ?>
