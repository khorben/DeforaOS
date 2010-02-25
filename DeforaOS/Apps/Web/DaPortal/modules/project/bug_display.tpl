<h1 class="title bug"><?php echo _html_safe($title); ?></h1>
<div class="bug">
<table>
	<tr><td class="field"><?php echo _html_safe(PROJECT); ?>:</td><td><a href="<?php echo _html_link('project', 'bug_list', $bug['project_id']); ?>"><?php echo _html_safe($bug['project']); ?></a></td><td class="field"><?php echo _html_safe(SUBMITTER); ?>:</td><td><a href="<?php echo _html_link('user', FALSE, $bug['user_id'], $bug['username']); ?>"><?php echo _html_safe($bug['username']); ?></a></td></tr>
	<tr><td class="field"><?php echo _html_safe(DATE); ?>:</td><td colspan="3"><?php echo _html_safe($bug['date']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><?php echo _html_safe($bug['state']); ?></td><td class="field"><?php echo _html_safe(TYPE); ?>:</td><td><?php echo _html_safe($bug['type']); ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(PRIORITY); ?>:</td><td><?php echo _html_safe($bug['priority']); ?></td><td class="field"><?php echo _html_safe(ASSIGNED_TO); ?>:</td><td><?php if(isset($bug['assigned_id']) && isset($bug['assigned'])) { ?><a href="<?php echo _html_link('user', FALSE, $bug['assigned_id'], $bug['assigned']); ?>"><?php echo _html_safe($bug['assigned']); ?></a><?php } ?></td></tr>
	<tr><td class="field"><?php echo _html_safe(DESCRIPTION); ?>:</td><td colspan="3"><?php echo str_replace("\r\n", "<br/>\n",_html_pre($bug['content'])); ?></td>
</table>
<?php if(isset($bug['id'])) { ?>
<form>
<a href="<?php echo _html_link('project', 'bug_reply', $bug['id']); ?>"><button type="button" class="icon reply"><?php echo _html_safe(REPLY); ?></button></a>
<?php if($admin == 1) { ?>
<a href="<?php echo _html_link('project', 'bug_modify', $bug['id'], $bug['title'], 'bug_id='.$bug['bug_id']); ?>"><button type="button" class="icon edit"><?php echo _html_safe(EDIT); ?></button></a></td></tr>
<?php } ?>
</form>
<?php } ?>
</div>
