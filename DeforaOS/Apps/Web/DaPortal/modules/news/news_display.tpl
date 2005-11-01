<div class="news entry">
	<div class="title"><span><span><span><? if($long) { ?><img src="modules/news/icon.png" alt=""/> <? } else { ?><a href="index.php?module=news&amp;id=<? echo _html_safe_link($news['id']); ?>"><? } ?><? echo _html_safe($news['title']); ?><? if(!$long) { ?></a><? } ?></span></span></span></div>
	<div class="author"><? echo _html_safe(NEWS_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe_link($news['user_id']); ?>"><? echo _html_safe($news['username']); ?></a></div>
	<div class="date"><? echo _html_safe(NEWS_ON); ?> <? echo _html_safe($news['date']); ?></div>
	<div class="content"><? echo _html_pre($news['content']); ?></div>
	<div class="status">
		<a href="index.php?module=news&amp;id=<? echo _html_safe_link($news['id']); ?>"><img src="icons/16x16/read.png" alt=""/> Read</a>
<? if(_module_id('comment')) { ?>
		(<? echo _html_safe(_module('comment', 'count', array('id' => $news['id'])).' '.COMMENT_S); ?>)
		· <a href="index.php?module=comment&amp;action=new&amp;parent=<? echo _html_safe($news['id']); ?>#edit"><img src="icons/16x16/reply.png" alt=""/> Reply</a>
<? } ?>
<? global $user_id; require_once('system/user.php');
if($news['id'] && _user_admin($user_id)) { ?>
		· <a href="index.php?module=news&amp;action=modify&amp;id=<? echo _html_safe($news['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <? echo _html_safe(EDIT); ?></a>
<? if($news['enabled'] == 't') { ?>
		· <a href="index.php?module=content&amp;action=disable&amp;id=<? echo _html_safe($news['id']); ?>&amp;show"><img src="icons/16x16/disabled.png" alt=""/> <? echo _html_safe(DISABLE); ?></a>
<? } else { ?>
		· <a href="index.php?module=content&amp;action=enable&amp;id=<? echo _html_safe($news['id']); ?>&amp;show"><img src="icons/16x16/enabled.png" alt=""/> <? echo _html_safe(ENABLE); ?></a>
<? } ?>
<? } ?>
	</div>
</div>
