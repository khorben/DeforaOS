<h1><img src="modules/category/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="category"/>
	<input type="hidden" name="action" value="<? echo isset($category) ? 'update' : 'insert'; ?>"/>
<? if(isset($category)) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($category['id']); ?>"/>
<? } ?>
	<table>
		<tr><td class="field"><? echo _html_safe(NAME); ?>:</td><td><input type="text" name="title" value="<? echo _html_safe($category['name']); ?>" size="50"/></td></tr>
		<tr><td class="field">Description:</td><td><input type="text" name="content" value="<? echo _html_safe($category['content']); ?>" size="50"/></td></tr>
		<tr><td></td><td><input type="submit" value="<? echo isset($category) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
