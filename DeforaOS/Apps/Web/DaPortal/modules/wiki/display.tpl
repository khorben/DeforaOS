<div class="wiki">
	<h1 class="title wiki"><?php echo _html_safe($title); ?></h1>
	<div class="content"><?php echo $wiki['content']; ?></div>
	<div><a href="<?php echo _html_link('wiki', 'modify', $wiki['id'], $wiki['title']); ?>"><div class="icon edit"></div><?php echo _html_safe(EDIT); ?></a></div>
</div>
