	<div class="entry">
		<div class="title"><span><span><span><?php echo $i; ?>. <a href="<?php echo _html_link($q['module'], '', $q['id'], $q['title']); ?>"><?php echo _html_safe($q['title']); ?></a></span></span></span></div>
		<div class="author">by <a href="<?php echo _html_link('user', '', $q['user_id'], $q['username']); ?>"><?php echo _html_safe($q['username']); ?></a></div>
		<div class="date">on <?php echo _html_safe($q['date']); ?></div>
		<div class="content"><?php echo _html_safe(substr($q['content'], 0, 400)); ?><?php if(strlen($q['content']) > 400) { ?>...<?php } ?></div>
	</div>
