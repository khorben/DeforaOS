<h1><img src="modules/project/report.png" alt=""/> <? echo _html_safe($title); ?></h1>
<div class="title"><? echo _html_safe($bug['title']); ?></div>
<table>
	<td></td><td><a href="index.php?module=project&amp;action=bug_modify&amp;<? echo _html_safe_link($bug['id']); ?>"><input type="button" value="Modifier"/></a></td>
	<td class="name"></td><td></td>
</table>
