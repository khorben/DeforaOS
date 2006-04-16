<hr/>
<div class="bug">
	<div class="title"><? echo _html_safe($reply['title']); ?></div>
	<div class="author"><? echo _html_safe($reply['username']); ?></div>
	<div><? echo _html_pre($reply['content']); ?></div>
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
