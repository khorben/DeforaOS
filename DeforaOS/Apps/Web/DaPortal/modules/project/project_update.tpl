<h1><img src="modules/project/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="<? echo isset($project) ? 'update' : 'insert'; ?>"/>
<? if(isset($project['id'])) { ?>
	<input type="hidden" name="id" value="<? echo _html_safe($project['id']); ?>"/>
<? } ?>
	<table>
		<tr><td class="field"><? echo _html_safe(NAME); ?>:</td><td><input type="text" name="name" value="<? echo _html_safe($project['name']); ?>" size="20"/></td></tr>
		<tr><td class="field">Short description:</td><td><input type="text" name="title" value="<? echo _html_safe($project['title']); ?>" size="80"/></td></tr>
		<tr><td class="field">Long description:</td><td><textarea name="content" cols="80" rows="10"><? echo _html_safe($project['content']); ?></textarea></td></tr>
		<tr><td class="field">CVS path:</td><td><input type="text" name="cvsroot" value="<? echo _html_safe($project['cvsroot']); ?>" size="80"/></td></tr>
		<tr><td></td><td><input type="submit" value="<? echo isset($project) ? _html_safe(UPDATE) : _html_safe(SEND); ?>"/></td></tr>
	</table>
</form>
