<?php if($long) { ?><h1 class="article"><?php echo _html_safe($article['title']); ?></h1><?php } ?>
<div class="article entry">
<?php if(!$long) { ?>
	<div class="title"><span><span><span><a href="index.php?module=article&amp;id=<?php echo _html_safe_link($article['id']); ?>"><?php echo _html_safe($article['title']); ?></a></span></span></span></div>
<?php } ?>
	<div class="author"><?php echo _html_safe(ARTICLE_BY); ?> <a href="index.php?module=user&amp;id=<?php echo _html_safe_link($article['user_id']); ?>"><?php echo _html_safe($article['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(ARTICLE_ON); ?> <?php echo _html_safe($article['date']); ?></div>
<?php if($long) { ?>
	<div class="content"><?php echo _html_pre($article['content']); ?></div>
<?php } ?>
	<div class="status">
<?php if(!isset($article['preview'])) { ?>
		<a href="index.php?module=article&amp;id=<?php echo _html_safe_link($article['id']); ?>"><img src="icons/16x16/read.png" alt=""/> Read</a>
<?php global $user_id; require_once('./system/user.php');
if($article['id'] && _user_admin($user_id)) { ?>
		· <a href="index.php?module=article&amp;action=modify&amp;id=<?php echo _html_safe($article['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <?php echo _html_safe(EDIT); ?></a>
<?php if($article['enabled'] == SQL_TRUE) { ?>
		· <a href="index.php?module=content&amp;action=disable&amp;id=<?php echo _html_safe($article['id']); ?>&amp;show"><img src="icons/16x16/disabled.png" alt=""/> <?php echo _html_safe(DISABLE); ?></a>
<?php } else { ?>
		· <a href="index.php?module=content&amp;action=enable&amp;id=<?php echo _html_safe($article['id']); ?>&amp;show"><img src="icons/16x16/enabled.png" alt=""/> <?php echo _html_safe(ENABLE); ?></a>
<?php } } } ?>
	</div>
</div>
