<form action="index.php" method="post">
	<input type="hidden" name="module" value="download"/>
	<input type="hidden" name="action" value="directory_insert"/>
<?php if(is_numeric($parent)) { ?>
	<input type="hidden" name="parent" value="<?php echo $parent; ?>"/>
<?php } ?>
	<table>
		<tr><td class="field">Name:</td><td><input type="text" name="title" value=""/></td></tr>
		<tr><td></td><td><input type="submit" value="Create"/></td></tr>
	</table>
</form>
