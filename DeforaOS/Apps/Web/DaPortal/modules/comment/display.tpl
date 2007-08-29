<div class="comment">
	<div class="container">
		<div class="title"><?php if(isset($comment['id'])) { ?><a href="<?php echo _html_link('comment', FALSE, $comment['id'], $comment['title']); ?>"><?php echo _html_safe($comment['title']); ?></a><?php } else { echo _html_safe($comment['title']); } ?></div>
		<div class="author"><?php echo _html_safe(COMMENT_BY); ?> <a href="<?php echo _html_link('user', FALSE, $comment['user_id'], $comment['username']); ?>"><?php echo _html_safe($comment['username']); ?></a></div>
		<div class="date"><?php echo _html_safe(COMMENT_ON); ?> <?php echo _html_safe($comment['date']); ?></div>
		<div class="content"><?php echo _html_pre($comment['content']); ?></div>
<?php if(!isset($comment['preview'])) { ?>
		<div class="status">
			<a href="<?php echo _html_link('comment', 'new', $comment['id'], FALSE, 'parent='.$comment['id']); ?>#edit"><div class="icon reply"></div> <?php echo _html_safe(REPLY); ?></a>
<?php global $user_id; require_once('./system/user.php');
if($comment['id'] && _user_admin($user_id)) { ?>
			&middot; <a href="index.php?module=content&amp;action=modify&amp;id=<?php echo _html_safe($comment['id']); ?>"><div class="icon edit"></div> <?php echo _html_safe(EDIT); ?></a>
<?php if($comment['enabled'] == SQL_TRUE) { ?>
			&middot; <a href="index.php?module=content&amp;action=disable&amp;id=<?php echo _html_safe($comment['id']); ?>&amp;show"><div class="icon disabled"></div> <?php echo _html_safe(DISABLE); ?></a>
<?php } else { ?>
			&middot; <a href="index.php?module=content&amp;action=enable&amp;id=<?php echo _html_safe($comment['id']); ?>&amp;show"><div class="icon enabled"></div> <?php echo _html_safe(ENABLE); ?></a>
<?php } ?>
<?php } ?>
		</div>
<?php } ?>
	</div>
<?php if(isset($comment['id'])) comment_childs(array('id' => $comment['id'])); ?>
</div>
