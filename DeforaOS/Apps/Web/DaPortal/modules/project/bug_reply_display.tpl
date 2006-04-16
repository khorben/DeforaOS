<div class="bug_reply">
	<div class="title"><? echo _html_safe($reply['title']); ?></div>
	<div class="author"><? echo _html_safe(REPLY_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe_link($reply['user_id']); ?>"><? echo _html_safe($reply['username']); ?></a></div>
	<div class="date"><? echo _html_safe(REPLY_ON.' '.$reply['date']); ?></div>
	<div><? echo _html_pre($reply['content']); ?></div>
<? if(strlen($reply['assigned'])) { ?>
	<div class="status"><? echo _html_safe(ASSIGNED_TO); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe_link($reply['assigned_id']); ?>"><? echo _html_safe($reply['assigned']); ?></a></div>
<? } ?>
<? if(isset($reply['state'])) { ?>
	<div class="status"><? echo _html_safe(STATE_CHANGED_TO.' "'.$reply['state'].'"'); ?></div>
<? } ?>
<? if(isset($reply['type'])) { ?>
	<div class="status"><? echo _html_safe(TYPE_CHANGED_TO.' "'.$reply['type'].'"'); ?></div>
<? } ?>
<? if(isset($reply['priority'])) { ?>
	<div class="status"><? echo _html_safe(PRIORITY_CHANGED_TO.' "'.$reply['priority'].'"'); ?></div>
<? } ?>
</div>
