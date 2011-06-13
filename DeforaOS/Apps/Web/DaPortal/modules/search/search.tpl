<h1 class="title search"><?php if(isset($_GET['q']) && strlen($_GET['q'])) echo _html_safe(SEARCH_RESULTS); else echo _html_safe(SEARCH); ?></h1>
<form action="index.php" method="get">
	<input type="hidden" name="module" value="search"/>
	<input type="text" name="q" value="<?php if(isset($_GET['q'])) echo _html_safe(stripslashes($_GET['q'])); ?>" size="30"/>
	<button type="submit" class="icon search"><?php echo _html_safe(SEARCH); ?></button><br/>
	<p><a href="<?php echo _html_link('search', 'advanced', FALSE, FALSE, isset($args['q']) ? array('q' => stripslashes($args['q'])) : FALSE); ?>"><?php echo _html_icon('add'); ?><?php echo _html_safe(ADVANCED_SEARCH).'...'; ?></a></p>
</form>
<hr/>
