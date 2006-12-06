<form action="index.php" method="post" enctype="multipart/form-data">
	<input type="hidden" name="module" value="download"/>
	<input type="hidden" name="action" value="file_insert"/>
<?php if(isset($parent) && is_numeric($parent)) { ?>
	<input type="hidden" name="parent" value="<?php echo $parent; ?>"/>
<?php } ?>
	<table>
		<tr><td class="field">File:</td><td><input type="file" name="file" value=""/></td></tr>
		<tr><td></td><td><input type="submit" value="Upload"/></td></tr>
	</table>
</form>
