<div class="article entry">
	<div class="title"><span><span><span><? if($long) { ?><img src="modules/article/icon.png" alt=""/> <? } else { ?><a href="index.php?module=article&amp;id=<? echo _html_safe_link($article['id']); ?>"><? } ?><? echo _html_safe($article['title']); ?><? if(!$long) { ?></a><? } ?></span></span></span></div>
	<div class="author"><? echo _html_safe(ARTICLE_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe_link($article['user_id']); ?>"><? echo _html_safe($article['username']); ?></a></div>
	<div class="date"><? echo _html_safe(ARTICLE_ON); ?> <? echo _html_safe($article['date']); ?></div>
<? if($long) { ?>
	<div class="content"><? echo _html_pre($article['content']); ?></div>
<? } ?>
	<div class="status">
<? if(!isset($article['preview'])) { ?>
		<a href="index.php?module=article&amp;id=<? echo _html_safe_link($article['id']); ?>"><img src="icons/16x16/read.png" alt=""/> Read</a>
<? global $user_id; require_once('system/user.php');
if($article['id'] && _user_admin($user_id)) { ?>
		· <a href="index.php?module=article&amp;action=modify&amp;id=<? echo _html_safe($article['id']); ?>"><img src="icons/16x16/edit.png" alt=""/> <? echo _html_safe(EDIT); ?></a>
<? if($article['enabled'] == 't') { ?>
		· <a href="index.php?module=content&amp;action=disable&amp;id=<? echo _html_safe($article['id']); ?>&amp;show"><img src="icons/16x16/disabled.png" alt=""/> <? echo _html_safe(DISABLE); ?></a>
<? } else { ?>
		· <a href="index.php?module=content&amp;action=enable&amp;id=<? echo _html_safe($article['id']); ?>&amp;show"><img src="icons/16x16/enabled.png" alt=""/> <? echo _html_safe(ENABLE); ?></a>
<? } } } ?>
	</div>
</div>
