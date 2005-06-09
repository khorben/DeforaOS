<h1><img src="modules/project/bug.png" alt=""/> Report a bug</h1>
<form action="index.php" method="post">
	<input type="hidden" name="module" value="project"/>
	<input type="hidden" name="action" value="bug_<? echo isset($bug) ? 'update' : 'insert'; ?>"/>
	<table>
		<tr><td class="field">Title:</td><td><input type="text" name="title" value="<? echo _html_safe($bug['title']); ?>" size="80"/></td></tr>
		<tr><td class="field">Description:</td><td><textarea name="content" cols="80" rows="20"><? echo _html_safe($bug['content']); ?></textarea></td></tr>
		<tr><td></td><td><input type="submit" value="Submit"/></td></tr>
	</table>
</form>
