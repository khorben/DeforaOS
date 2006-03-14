<h1><img src="" alt=""/> <? echo _html_safe($bookmark['title']); ?></h1>
<p><? echo _html_safe($bookmark['content']); ?>
<p><a href="<? echo _html_safe_link($bookmark['url']); ?>"></a><? echo _html_safe($bookmark['url']); ?></p>
<p><a href="index.php?module=bookmark&amp;action=modify&amp;id=<? echo $bookmark['id']; ?>"><button><? echo _html_safe(MODIFY); ?></button></a></p>
