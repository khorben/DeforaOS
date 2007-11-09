<form action="index.php" method="post">
	<input type="hidden" name="module" value="papadam"/>
	<input type="hidden" name="action" value="download"/>
	<table>
		<tr><td class="field"></td><td><i>hostname ":" filename</i></td></tr>
		<tr><td class="field"><?php echo _html_safe(ADDRESS); ?>:</td><td><input type="text" name="address" value="<?php echo _html_safe($address); ?>" size="20"/></td></tr>
		<tr><td></td><td><input type="submit" value="<?php echo _html_safe(DOWNLOAD); ?>"/></td></tr>
	</table>
</form>
