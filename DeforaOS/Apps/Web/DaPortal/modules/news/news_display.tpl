<? if($long) { ?><h1><img src="modules/news/icon.png" alt=""/> <? echo _html_safe($title); ?></h1><? } ?>
<div class="news entry">
	<div class="title"><span><span><span><? if(!$long) { ?><a href="index.php?module=news&amp;id=<? echo _html_safe_link($news['id']); ?>"><? } ?><? echo _html_safe($news['title']); ?><? if(!$long) { ?></a><? } ?></span></span></span></div>
	<div class="author"><? echo _html_safe(NEWS_BY); ?> <a href="index.php?module=user&amp;id=<? echo _html_safe_link($news['user_id']); ?>"><? echo _html_safe($news['username']); ?></a></div>
	<div class="date"><? echo _html_safe(NEWS_ON); ?> <? echo _html_safe($news['date']); ?></div>
<? if($news['id'] && _user_admin($user_id)) { ?>
	<div class="edit"><a href="index.php?module=news&amp;action=modify&amp;id=<? echo _html_safe($news['id']); ?>"><? echo _html_safe(EDIT); ?></a></div>
<? } ?>
	<div class="content"><? echo _html_pre($news['content']); ?></div>
</div>
