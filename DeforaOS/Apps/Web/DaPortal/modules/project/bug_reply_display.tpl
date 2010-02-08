<div class="bug_reply">
	<div class="title"><?php echo _html_safe($reply['title']); ?></div>
	<div class="author"><?php echo _html_safe(REPLY_BY); ?> <a href="<?php echo _html_link('user', FALSE, $reply['user_id'], $reply['username']); ?>"><?php echo _html_safe($reply['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(REPLY_ON.' '.$reply['date']); ?></div>
	<div class="content"><?php echo _html_pre($reply['content']); ?></div>
<?php if(isset($reply['assigned_id']) && isset($reply['assigned'])) { ?>
	<div class="status"><?php echo _html_safe(ASSIGNED_TO); ?> <a href="<?php echo _html_link('user', FALSE, $reply['assigned_id'], $reply['assigned']); ?>"><?php echo _html_safe($reply['assigned']); ?></a></div>
<?php } ?>
<?php if(isset($reply['state'])) { ?>
	<div class="status"><?php echo _html_safe(STATE_CHANGED_TO.' "'.$reply['state'].'"'); ?></div>
<?php } ?>
<?php if(isset($reply['type'])) { ?>
	<div class="status"><?php echo _html_safe(TYPE_CHANGED_TO.' "'.$reply['type'].'"'); ?></div>
<?php } ?>
<?php if(isset($reply['priority'])) { ?>
	<div class="status"><?php echo _html_safe(PRIORITY_CHANGED_TO.' "'.$reply['priority'].'"'); ?></div>
<?php } ?>
<?php if($admin && isset($reply['content_id'])) { ?>
	<div><a href="<?php echo _html_link('project', 'bug_reply_modify', $reply['id']); ?>"><button class="icon edit"><?php echo _html_safe(EDIT); ?></button></a></div>
<?php } ?>
</div>
