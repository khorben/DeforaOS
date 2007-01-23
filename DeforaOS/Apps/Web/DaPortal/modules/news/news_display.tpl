<div class="news entry">
	<div class="title"><span><span><span><?php if(!$long) { ?><a href="index.php?module=news&amp;id=<?php echo _html_safe_link($news['id']); ?>"><?php } ?><?php echo _html_safe($news['title']); ?><?php if(!$long) { ?></a><?php } ?></span></span></span></div>
	<div class="author"><?php echo _html_safe(NEWS_BY); ?> <a href="index.php?module=user&amp;id=<?php echo _html_safe_link($news['user_id']); ?>"><?php echo _html_safe($news['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(NEWS_ON); ?> <?php echo _html_safe($news['date']); ?></div>
	<div class="content"><?php echo _html_pre($news['content']); ?></div>
	<div class="status">
<?php if(isset($news['preview'])) { ?>
		<?php echo _html_safe(NEWS_PREVIEW); ?>
<?php } else { ?>
		<a href="index.php?module=news&amp;id=<?php echo _html_safe_link($news['id']); ?>"><div class="icon read"></div> <?php echo _html_safe(READ); ?></a>
<?php if(_module_id('comment')) { ?>
		(<?php echo _html_safe(_module('comment', 'count', array('id' => $news['id'])).' '.COMMENT_S); ?>)
		&middot; <a href="index.php?module=comment&amp;action=new&amp;parent=<?php echo _html_safe($news['id']); ?>#edit"><div class="icon reply"></div> <?php echo _html_safe(REPLY); ?></a>
<?php } } ?>
<?php global $user_id; require_once('./system/user.php');
if(isset($news['id']) && _user_admin($user_id)) { ?>
		&middot; <a href="index.php?module=news&amp;action=modify&amp;id=<?php echo _html_safe($news['id']); ?>"><div class="icon edit"></div> <?php echo _html_safe(EDIT); ?></a>
<?php if(isset($news['enabled'])) if($news['enabled'] == SQL_TRUE) { ?>
		&middot; <a href="index.php?module=content&amp;action=disable&amp;id=<?php echo _html_safe($news['id']); ?>&amp;show"><div class="icon disabled"></div> <?php echo _html_safe(DISABLE); ?></a>
<?php } else { ?>
		&middot; <a href="index.php?module=content&amp;action=enable&amp;id=<?php echo _html_safe($news['id']); ?>&amp;show"><div class="icon enabled"></div> <?php echo _html_safe(ENABLE); ?></a>
<?php } ?>
<?php } ?>
	</div>
</div>
