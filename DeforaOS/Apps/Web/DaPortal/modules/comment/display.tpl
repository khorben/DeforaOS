<div class="comment">
	<div class="container">
		<div class="title"><?php if(isset($comment['id'])) { ?><a href="<?php echo _html_link($comment['module'], FALSE, $comment['parent']).'#'.$comment['id']; ?>"><?php echo _html_safe($comment['title']); ?></a><?php } else { echo _html_safe($comment['title']); } ?></div>
		<div class="author"><?php echo _html_safe(COMMENT_BY); ?> <a href="<?php echo _html_link('user', FALSE, $comment['user_id'], $comment['username']); ?>"><?php echo _html_safe($comment['username']); ?></a></div>
		<div class="date"><?php echo _html_safe(COMMENT_ON); ?> <?php echo _html_safe($comment['date']); ?></div>
		<div class="content"><?php echo _html_pre($comment['content']); ?></div>
<?php if(!isset($comment['preview'])) { ?>
		<div class="status">
			<a href="<?php echo _html_link($comment['module'], 'reply', $comment['parent'], FALSE, 'parent='.$comment['id']); ?>#edit"><span class="icon reply"></span> <?php echo _html_safe(REPLY); ?></a>
<?php global $user_id; require_once('./system/user.php');
if($comment['id'] && _user_admin($user_id)) { ?>
			<span class="middot">&middot;</span>
			<a href="<?php echo _html_link('content', 'update', $comment['id']); ?>"><span class="icon edit"></span> <?php echo _html_safe(EDIT); ?></a>
<?php } ?>
		</div>
<?php } ?>
	</div>
<?php //if(isset($comment['id'])) comment_childs(array('id' => $comment['id'])); ?>
</div>
