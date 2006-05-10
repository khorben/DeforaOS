<h1><img src="modules/project/bug.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<table>
	<tr><td class="field"><?php echo _html_safe(PROJECT); ?>:</td><td><a href="index.php?module=project&amp;action=bug_list&amp;project_id=<?php echo _html_safe($bug['project_id']); ?>"><?php echo _html_safe($bug['project']); ?></a></td><td class="field"><?php echo _html_safe(SUBMITTER); ?>:</td><td><a href="index.php?module=user&amp;id=<?php echo _html_safe($bug['user_id']); ?>"><?php echo _html_safe($bug['username']); ?></a></td></tr>
	<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><?php echo _html_safe($bug['state']); ?></td><td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><?php echo _html_safe($bug['type']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(PRIORITY); ?>:</td><td><?php echo _html_safe($bug['priority']); ?></td><td class="field"><?php echo _html_safe(ASSIGNED_TO); ?>:</td><td><a href="index.php?module=user&amp;id=<?php echo _html_safe($bug['assigned_id']); ?>"><?php echo _html_safe($bug['assigned']); ?></a></td></tr>
	<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td colspan="3"><?php echo str_replace("\r\n", "<br/>\n",_html_pre($bug['content'])); ?></td>
</table>
<a href="index.php?module=project&amp;action=bug_reply&amp;id=<?php echo $bug['id']; ?>"><img src="icons/16x16/reply.png" alt=""/> <?php echo _html_safe(REPLY); ?></a>
<?php if($admin) { ?>
· <a href="index.php?module=project&amp;action=bug_modify&amp;id=<?php echo _html_safe_link($bug['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <?php echo _html_safe(EDIT); ?></a></td></tr>
<?php } ?>
