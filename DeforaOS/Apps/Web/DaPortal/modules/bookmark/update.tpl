<h1><img src="modules/bookmark/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="bookmark"/>
	<input type="hidden" name="action" value="<? echo isset($bookmark) ? 'update' : 'insert'; ?>"/>
<? if(isset($bookmark['id'])) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($bookmark['id']); ?>"/>
<? } ?>
	<table>
		<tr><td class="field"><? echo _html_safe(TITLE); ?>:</td><td><input type="text" name="title" value="<? echo _html_safe($bookmark['title']); ?>" size="50"/></td></tr>
		<tr><td class="field"><? echo _html_safe(ADDRESS); ?>:</td><td><input type="text" name="url" value="<? echo _html_safe($bookmark['url']); ?>" size="50"/></td></tr>
		<tr><td class="field">Description:</td><td><textarea name="content" cols="50" rows="30"><? echo _html_safe($bookmark['content']); ?></textarea></td></tr>
		<tr><td class="field"><? echo _html_safe(PUBLIC); ?>:</td><td><input type="checkbox" name="enabled"<? if($bookmark['enabled'] == 't') { ?> checked="checked"<? } ?>/></td></tr>
		<tr><td></td><td><input type="submit" value="<? echo isset($bookmark) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
