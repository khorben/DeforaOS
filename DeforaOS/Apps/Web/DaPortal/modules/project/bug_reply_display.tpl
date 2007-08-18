<div class="bug_reply">
	<div class="title"><?php echo _html_safe($reply['title']); ?></div>
	<div class="author"><?php echo _html_safe(REPLY_BY); ?> <a href="index.php?module=user&amp;id=<?php echo _html_safe($reply['user_id']); ?>"><?php echo _html_safe($reply['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(REPLY_ON.' '.$reply['date']); ?></div>
	<div><?php echo _html_pre($reply['content']); ?></div>
<?php if(strlen($reply['assigned'])) { ?>
	<div class="status"><?php echo _html_safe(ASSIGNED_TO); ?> <a href="index.php?module=user&amp;id=<?php echo _html_safe($reply['assigned_id']); ?>"><?php echo _html_safe($reply['assigned']); ?></a></div>
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
	<div><a href="index.php?module=project&amp;action=bug_reply_modify&amp;id=<?php echo _html_safe($reply['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <?php echo _html_safe(EDIT); ?></a> &middot; <a href="index.php?module=content&amp;action=disable&amp;id=<?php echo _html_safe($reply['content_id']); ?>"><img src="icons/16x16/disabled.png" alt=""/> <?php echo _html_safe(DISABLE); ?></a></div>
<?php } ?>
</div>
