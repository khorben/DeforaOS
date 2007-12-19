<form action="index.php" method="get">
	<input type="hidden" name="module" value="wiki"/>
	<table>
		<tr><td class="field">Look for a page:</td><td><input type="text" name="title"<?php if(isset($title)) { ?> value="<?php echo _html_safe($title); ?>"<?php } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="Search"/></td></tr>
	</table>
</form>
