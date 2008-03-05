<?php if(isset($title)) { ?>
<h1 class="title news"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<div class="news entry">
<?php if(isset($news['title'])) { ?>
	<div class="title"><span><span><span><?php if(!$long) { ?><a href="<?php echo _html_link('news', FALSE, $news['id'], $news['tag']); ?>"><?php } ?><?php echo _html_safe($news['title']); ?><?php if(!$long) { ?></a><?php } ?></span></span></span></div>
<?php } ?>
	<div class="author"><?php echo _html_safe(NEWS._BY_); ?> <a href="<?php echo _html_link('user', FALSE, $news['user_id'], $news['username']); ?>"><?php echo _html_safe($news['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(NEWS_ON); ?> <?php echo _html_safe($news['date']); ?></div>
	<div class="content"><?php echo _html_pre($news['content']); ?></div>
	<div class="status">
<?php if(isset($news['preview'])) { ?>
		<?php echo _html_safe(NEWS_PREVIEW); ?>
<?php } else { ?>
		<a href="<?php echo _html_link('news', FALSE, $news['id'], $news['tag']); ?>"><div class="icon read"></div> <?php echo _html_safe(READ); ?></a>
<?php if(_module_id('comment')) { ?>
		(<?php echo _html_safe(_module('comment', 'count', array('id' => $news['id'])).' '.COMMENT_S); ?>)
		<span class="middot">&middot;</span>
		<a href="<?php echo _html_link('news', 'reply', $news['id']); ?>#edit"><div class="icon reply"></div> <?php echo _html_safe(REPLY); ?></a>
<?php } } ?>
<?php global $user_id; require_once('./system/user.php');
if(isset($news['id']) && _user_admin($user_id)) { ?>
		<span class="middot">&middot;</span>
		<a href="<?php echo _html_link('news', 'update', $news['id']); ?>"><div class="icon edit"></div> <?php echo _html_safe(EDIT); ?></a>
<?php } ?>
	</div>
</div>
