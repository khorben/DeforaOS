<div class="comment">
	<div class="container">
		<div class="title"><?php if(isset($comment['id'])) { ?><a href="index.php?module=comment&amp;id=<?php echo _html_safe($comment['id']); ?>"><?php echo _html_safe($comment['title']); ?></a><?php } else { echo _html_safe($comment['title']); } ?></div>
		<div class="author"><?php echo _html_safe(COMMENT_BY); ?> <a href="index.php?module=user&amp;id=<?php echo _html_safe($comment['user_id']); ?>"><?php echo _html_safe($comment['username']); ?></a></div>
		<div class="date"><?php echo _html_safe(COMMENT_ON); ?> <?php echo _html_safe($comment['date']); ?></div>
		<div class="content"><?php echo _html_pre($comment['content']); ?></div>
<?php if(!isset($comment['preview'])) { ?>
		<div class="status">
			<a href="index.php?module=comment&amp;action=new&amp;parent=<?php echo _html_safe($comment['id']); ?>#edit"><img src="icons/16x16/reply.png" alt=""/> Reply</a>
<?php global $user_id; require_once('./system/user.php');
if($comment['id'] && _user_admin($user_id)) { ?>
			· <a href="index.php?module=content&amp;action=modify&amp;id=<?php echo _html_safe($comment['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <?php echo _html_safe(EDIT); ?></a>
<?php if($comment['enabled'] == SQL_TRUE) { ?>
			· <a href="index.php?module=content&amp;action=disable&amp;id=<?php echo _html_safe($comment['id']); ?>&amp;show"><img src="icons/16x16/disabled.png" alt=""/> <?php echo _html_safe(DISABLE); ?></a>
<?php } else { ?>
			· <a href="index.php?module=content&amp;action=enable&amp;id=<?php echo _html_safe($comment['id']); ?>&amp;show"><img src="icons/16x16/enabled.png" alt=""/> <?php echo _html_safe(ENABLE); ?></a>
<?php } ?>
<?php } ?>
		</div>
<?php } ?>
	</div>
<?php if(isset($comment['id'])) comment_childs(array('id' => $comment['id'])); ?>
</div>
