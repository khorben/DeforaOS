<h1 class="title download"><?php echo _html_safe(DOWNLOADS.': '.$file['name']); ?></h1>
<p>
	<a href="<?php echo _html_link('download', FALSE, FALSE, FALSE, 'download_id='.$file['parent']); ?>"><div class="icon parent_directory"></div>Browse</a>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('download', 'download', $file['id']); ?>"><div class="icon download"></div>Download</a>
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
<h2><?php echo _html_safe(IMAGE_PREVIEW); ?></h2>
<a href="<?php echo _html_link('download', 'download', $file['id']); ?>" title="<?php echo _html_safe($file['name']); ?>"><img src="<?php echo _html_link('download', 'download', $file['id']); ?>" alt="" style="max-height: 100px; max-width: 100px"/></a>
<?php } ?>
