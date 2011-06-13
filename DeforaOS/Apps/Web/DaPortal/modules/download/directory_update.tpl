<form action="<?php echo _html_link(); ?>" method="post">
	<input type="hidden" name="module" value="download"/>
	<input type="hidden" name="action" value="directory_insert"/>
<?php if(is_numeric($parent)) { ?>
	<input type="hidden" name="parent" value="<?php echo $parent; ?>"/>
<?php } ?>
	<table>
		<tr><td class="field"><?php echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value=""/></td></tr>
		<tr><td></td><td><a href="<?php echo _html_link('download', FALSE, $parent); ?>"><button type="button" class="icon cancel"><?php echo _html_safe(CANCEL); ?></button></a> <input type="submit" value="<?php echo _html_safe(CREATE); ?>" class="icon folder-new"/></td></tr>
	</table>
</form>
