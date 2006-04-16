<h1><img src="modules/project/bug.png" alt=""/> <? echo _html_safe($title); ?></h1>
<table>
	<tr><td class="field"><? echo _html_safe(PROJECT); ?>:</td><td><a href="index.php?module=project&amp;id=<? echo _html_safe($bug['project_id']); ?>"><? echo _html_safe($bug['project']); ?></a></td><td class="field"><? echo _html_safe(SUBMITTER); ?>:</td><td><a href="index.php?module=user&amp;id=<? echo _html_safe($bug['user_id']); ?>"><? echo _html_safe($bug['username']); ?></a></td></tr>
	<tr><td class="field"><? echo _html_safe(STATE); ?>:</td><td><? echo _html_safe($bug['state']); ?></td><td class="field"><? echo _html_safe(TYPE); ?>:</td><td><? echo _html_safe($bug['type']); ?></td></tr>
	<tr><td class="field"><? echo _html_safe(PRIORITY); ?>:</td><td><? echo _html_safe($bug['priority']); ?></td><td class="field"><? echo _html_safe(ASSIGNED_TO); ?>:</td><td><a href="index.php?module=user&amp;id=<? echo _html_safe($bug['assigned_id']); ?>"><? echo _html_safe($bug['assigned']); ?></a></td></tr>
	<tr><td class="field"><? echo _html_safe(DESCRIPTION); ?>:</td><td colspan="3"><? echo str_replace("\r\n", "<br/>\n",_html_pre($bug['content'])); ?></td>
</table>
<a href="index.php?module=project&amp;action=bug_reply&amp;id=<? echo $bug['id']; ?>"><img src="icons/16x16/reply.png" alt=""/> <? echo _html_safe(REPLY); ?></a>
<? if($admin) { ?>
· <a href="index.php?module=project&amp;action=bug_modify&amp;id=<? echo _html_safe_link($bug['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <? echo _html_safe(EDIT); ?></a></td></tr>
<? } ?>
