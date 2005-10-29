<div class="comment">
	<div class="container">
		<div class="title"><a href="index.php?module=comment&amp;id=<? echo _html_safe($comment['id']); ?>"><? echo _html_safe($comment['title']); ?></a></div>
		<div class="author"><? echo _html_safe(COMMENT_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe($comment['user_id']); ?>"><? echo _html_safe($comment['username']); ?></a></div>
		<div class="date"><? echo _html_safe(COMMENT_ON); ?> <? echo _html_safe($comment['date']); ?></div>
		<div class="content"><? echo _html_pre($comment['content']); ?></div>
		<div class="status">
			<a href="index.php?module=comment&amp;action=new&amp;parent=<? echo _html_safe($comment['id']); ?>#edit">Reply</a>
<? global $user_id; require_once('system/user.php');
if($comment['id'] && _user_admin($user_id)) { ?>
			· <a href="index.php?module=content&amp;action=modify&amp;id=<? echo _html_safe($comment['id']); ?>"><? echo _html_safe(EDIT); ?></a>
<? if($comment['enabled'] == 't') { ?>
			· <a href="index.php?module=content&amp;action=disable&amp;id=<? echo _html_safe($comment['id']); ?>&amp;show"><? echo _html_safe(DISABLE); ?></a>
<? } else { ?>
			· <a href="index.php?module=content&amp;action=enable&amp;id=<? echo _html_safe($comment['id']); ?>&amp;show"><? echo _html_safe(ENABLE); ?></a>
<? } ?>
<? } ?>
		</div>
	</div>
<? if(isset($comment['id'])) comment_childs(array('id' => $comment['id'])); ?>
</div>
