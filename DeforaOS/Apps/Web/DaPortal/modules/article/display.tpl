<?php if($long) { ?><h1 class="title article"><?php echo _html_safe($article['title']); ?></h1><?php } ?>
<div class="article entry">
<?php if(!$long) { ?>
	<div class="title"><span><span><span><a href="<?php echo _html_link('article', '', $article['id'], $article['tag']); ?>"><?php echo _html_safe($article['title']); ?></a></span></span></span></div>
<?php } ?>
	<div class="author"><?php echo _html_safe(ARTICLE._BY_); ?> <a href="<?php echo _html_link('user', FALSE, $article['user_id'], $article['username']); ?>"><?php echo _html_safe($article['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(ARTICLE_ON); ?> <?php echo _html_safe($article['date']); ?></div>
<?php if($long) { ?>
	<div class="content"><?php echo _html_pre($article['content']); ?></div>
<?php } ?>
	<div class="status">
<?php if(!isset($article['preview'])) { ?>
		<a href="<?php echo _html_link('article', FALSE, $article['id'], $article['tag']); ?>"><?php echo _html_icon('read'); ?><?php echo _html_safe(READ); ?></a>
<?php global $user_id; require_once('./system/user.php');
if($article['id'] && _user_admin($user_id)) { ?>
		<span class="middot">&middot;</span>
		<a href="<?php echo _html_link('article', 'modify', $article['id']); ?>"><?php echo _html_icon('edit'); ?><?php echo _html_safe(EDIT); ?></a>
<?php } } ?>
	</div>
</div>
