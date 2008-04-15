<h1 class="title download"><?php echo _html_safe(DOWNLOADS.': '.$file['name']); ?></h1>
<p>
	<a href="<?php echo _html_link('download', FALSE, FALSE, FALSE, 'download_id='.$file['parent']); ?>"><div class="icon parent_directory"></div>Browse</a>
	<span class="middot">&middot;</span>
	<a href="<?php echo _html_link('download', 'download', $file['id']); ?>"><div class="icon download"></div><?php echo _html_safe(DOWNLOAD); ?></a>
</p>
<table>
	<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><?php echo _html_safe($file['name']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><?php echo _html_safe($file['mime']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(OWNER); ?>:</td><td><a href="<?php echo _html_link('user', FALSE, $file['user_id'], $file['user']); ?>"><?php echo _html_safe($file['user']); ?></a></td></tr>
	<tr><td class="field"><?php echo _html_safe(PERMISSIONS); ?>:</td><td style="font-family: monospace"><?php echo _html_safe($file['mode']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(CREATION_TIME); ?>:</td><td><?php echo _html_safe($file['ctime']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(MODIFICATION_TIME); ?>:</td><td><?php echo _html_safe($file['mtime']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(ACCESS_TIME); ?>:</td><td><?php echo _html_safe($file['atime']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(SIZE); ?>:</td><td><?php echo _html_safe($file['size']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(COMMENT); ?>:</td><td><?php echo _html_pre($file['content']); ?></td></tr>
</table>
<?php if(strncmp('image/', $file['mime'], 6) == 0) { ?>
<h2><?php echo _html_safe(IMAGE_PREVIEW); ?></h2>
<a href="<?php echo _html_link('download', 'download', $file['id'], $file['name']); ?>" title="<?php echo _html_safe($file['name']); ?>"><img src="<?php echo _html_link('download', 'download', $file['id'], $file['name']); ?>" alt="" style="max-height: 100px; max-width: 100px"/></a>
<?php } ?>
