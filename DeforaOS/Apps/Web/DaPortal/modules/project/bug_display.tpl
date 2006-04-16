<h1><img src="modules/project/bug.png" alt=""/> <? echo _html_safe($title); ?></h1>
<table>
	<tr><td class="field">Project:</td><td><a href="index.php?module=project&amp;id=<? echo _html_safe($bug['project_id']); ?>"><? echo _html_safe($bug['project']); ?></a></td><td class="field">Submitter:</td><td><a href="index.php?module=user&amp;id=<? echo _html_safe($bug['user_id']); ?>"><? echo _html_safe($bug['username']); ?></a></td></tr>
	<tr><td class="field">State:</td><td><? echo _html_safe($bug['state']); ?></td><td class="field">Type:</td><td><? echo _html_safe($bug['type']); ?></td></tr>
	<tr><td class="field">Priority:</td><td><? echo _html_safe($bug['priority']); ?></td><td></td><td></td></tr>
	<tr><td class="field">Description:</td><td colspan="3"><? echo str_replace("\r\n", "<br/>\n",_html_pre($bug['content'])); ?></td>
<? if($admin) { ?>
	<tr><td></td><td colspan="3"><a href="index.php?module=project&amp;action=bug_modify&amp;id=<? echo _html_safe_link($bug['id']); ?>"><input type="button" value="Modify"/></a></td></tr>
<? } ?>
</table>
<a href="index.php?module=project&amp;action=bug_reply&amp;id=<? echo $bug['id']; ?>"><img src="icons/16x16/reply.png" alt=""/> Reply</a>
