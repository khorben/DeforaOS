<h1><img src="modules/search/icon.png" alt=""/> <?php if(strlen($_GET['q'])) echo _html_safe(SEARCH_RESULTS); else echo _html_safe(SEARCH); ?></h1>
<form action="index.php" method="get">
	<input type="hidden" name="module" value="search"/>
	<input type="text" name="q" value="<?php echo _html_safe(stripslashes($_GET['q'])); ?>" size="30"/>
	<input type="submit" value="<?php echo _html_safe(SEARCH); ?>"/>
</form>
