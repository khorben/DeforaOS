	<div class="entry">
		<div class="title"><? echo $i; ?>. <a href="index.php?module=<? echo _html_safe_link($q['module']); ?>&id=<? echo _html_safe_link($q['id']); ?>"><? echo _html_safe($q['title']); ?></a></div>
		<div class="author">by <a href="index.php?module=user&id=<? echo _html_safe_link($q['user_id']); ?>"><? echo _html_safe($q['username']); ?></a></div>
		<div class="date">on <? echo _html_safe($q['date']); ?></div>
		<div class="content"><? echo _html_safe(substr($q['content'], 0, 400)); ?><? if(strlen($q['content']) > 400) { ?>...<? } ?></div>
	</div>
