<?
	$tags = array('br/', 'li', '/li', 'p', '/p', 'ul', '/ul');
	$news['content_display'] = _html_safe($news['content']);
	foreach($tags as $t)
		$news['content_display'] = str_replace("&lt;$t&gt;",
				"<$t>", $news['content_display']);
?>
<? if($long) { ?><h1><img src="modules/news/icon.png" alt=""/> <? echo _html_safe($title); ?></h1><? } ?>
<div class="news entry">
	<div class="title"><? if(!$long) { ?><a href="index.php?module=news&id=<? echo _html_safe($news['id']); ?>"><? } ?><? echo _html_safe($news['title']); ?><? if(!$long) { ?></a><? } ?></div>
	<div class="author">by <a href="index.php?module=user&id=<? echo _html_safe($news['user_id']); ?>"><? echo _html_safe($news['username']); ?></a></div>
	<div class="date">on <? echo _html_safe($news['date']); ?></div>
<? /*	<div class="content"><? if($long) echo _html_safe($news['content_display']); else { echo _html_safe(substr($news['content'], 0, 400)); if(strlen($news['content']) > 400) echo '...'; } ?></div> */ ?>
	<div class="content"><? echo $news['content_display']; ?></div>
</div>
