	<div class="entry">
		<div class="title"><span><span><span><?php echo $i; ?>. <a href="<?php echo _html_link($q['module'], FALSE, $q['id'], $q['title']); ?>"><?php echo (isset($from) && isset($to)) ? str_ireplace($from, $to, _html_safe($q['title'])) : _html_safe($q['title']); ?></a></span></span></span></div>
		<div class="author">by <a href="<?php echo _html_link('user', FALSE, $q['user_id'], $q['username']); ?>"><?php echo _html_safe($q['username']); ?></a></div>
		<div class="date">on <?php echo _html_safe($q['date']); ?></div>
		<div class="content"><?php echo (isset($from) && isset($to)) ? str_ireplace($from, $to, _html_safe($q['content'])) : _html_safe($q['content']); ?></div>
	</div>
