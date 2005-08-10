<? if($long) { ?><h1><img src="modules/news/icon.png" alt=""/> <? echo _html_safe($title); ?></h1><? } ?>
<div class="news entry">
	<div class="title"><? if(!$long) { ?><a href="index.php?module=news&id=<? echo _html_safe_link($news['id']); ?>"><? } ?><? echo _html_safe($news['title']); ?><? if(!$long) { ?></a><? } ?></div>
	<div class="author"><? echo _html_safe(NEWS_BY); ?> <a href="index.php?module=user&id=<? echo _html_safe_link($news['user_id']); ?>"><? echo _html_safe($news['username']); ?></a></div>
	<div class="date"><? echo _html_safe(NEWS_ON); ?> <? echo _html_safe($news['date']); ?></div>
	<div class="content"><? echo _html_tags($news['content']); ?></div>
</div>
