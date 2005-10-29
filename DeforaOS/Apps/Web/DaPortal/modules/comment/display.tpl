<div class="comment">
	<div class="title"><a href="index.php?module=comment&amp;id=<? echo _html_safe($comment['id']); ?>"><? echo _html_safe($comment['title']); ?></a></div>
	<div class="author"><? echo _html_safe(COMMENT_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe($comment['user_id']); ?>"><? echo _html_safe($comment['username']); ?></a></div>
	<div class="date"><? echo _html_safe(COMMENT_ON); ?> <? echo _html_safe($comment['date']); ?></div>
	<a href="index.php?module=comment&amp;action=new&amp;parent=<? echo _html_safe($comment['id']); ?>#edit">Reply</a>
	<div class="content"><? echo _html_safe($comment['content']); ?></div>
<? if(isset($comment['id'])) comment_childs(array('id' => $comment['id'])); ?>
</div>
