<?php if(isset($file)) { ?>
<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="download"/>
	<input type="hidden" name="action" value="file_update"/>
	<input type="hidden" name="id" value="<?php echo $file['id']; ?>"/>
<?php } else { ?>
<form action="<?php echo _html_link(); ?>" method="post" enctype="multipart/form-data">
	<input type="hidden" name="module" value="download"/>
	<input type="hidden" name="action" value="file_insert"/>
<?php } ?>
<?php if(isset($parent) && is_numeric($parent)) { ?>
	<input type="hidden" name="parent" value="<?php echo $parent; ?>"/>
<?php } ?>
	<table>
<?php if(isset($file)) { ?>
		<tr><td class="field"><?php echo _html_safe(FILE); ?>:</td><td><input type="text" name="file" value="<?php echo _html_safe($file['name']); ?>" size="30"/></td></tr>
<?php } else { ?>
		<tr><td class="field"><?php echo _html_safe(FILE); ?>:</td><td><input type="file" name="file" value="" size="30"/></td></tr>
<?php } ?>
		<tr><td class="field"><?php echo _html_safe(COMMENT); ?>:</td><td><textarea name="comment" value="" cols="35" rows="4"><?php if(isset($file)) echo _html_safe($file['comment']); ?></textarea></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('download', FALSE, isset($file) ? $file['id'] : FALSE, isset($file) ? $file['name'] : FALSE, isset($file) ? FALSE : array('download_id' => $parent)); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <button type="reset" class="icon reset"><?php echo _html_safe(RESET); ?></button> <input type="submit" value="<?php echo isset($file) ? _html_safe(UPDATE) : _html_safe(UPLOAD); ?>" class="icon submit"/></td></tr>
	</table>
</form>
