<h1><img src="modules/project/icon.png" alt=""/> New project</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="<? echo isset($project) ? 'update' : 'insert'; ?>"/>
	<table>
		<tr><td class="field">Name:</td><td><input type="text" name="name" value="<? echo _html_safe($project['name']); ?>" size="20"/></td></tr>
		<tr><td class="field">Short description:</td><td><input type="text" name="title" value="<? echo _html_safe($project['title']); ?>" size="80"/></td></tr>
		<tr><td class="field">Long description:</td><td><textarea name="content" cols="80" rows="20"><? echo _html_safe($project['content']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="Send"/></td></tr>
	</table>
</form>
