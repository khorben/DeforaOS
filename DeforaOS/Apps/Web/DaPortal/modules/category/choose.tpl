<form action="index.php" method="post">
	<input type="hidden" name="module" value="category"/>
	<input type="hidden" name="action" value="link_insert_new"/>
	<input type="hidden" name="content_id" value="<? echo _html_safe($args['id']); ?>"/>
	<span class="field">Or enter one directly:</span> <input type="text" name="title" size="15"/> <input type="submit" value="Add"/>
</form>
