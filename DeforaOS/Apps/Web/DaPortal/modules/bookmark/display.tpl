<h1><img src="modules/bookmark/icon.png" alt=""/> <? echo _html_safe($bookmark['title']); ?></h1>
<table>
	<tr><td class="field"><? echo _html_safe(TITLE); ?>:</td><td><? echo _html_safe($bookmark['title']); ?></td></tr>
	<tr><td class="field"><? echo _html_safe(ADDRESS); ?>:</td><td><a href="<? echo _html_safe_link($bookmark['url']); ?>"><? echo _html_safe($bookmark['url']); ?></a></td></tr>
<? if($user_id == $bookmark['user_id']) { ?>
	<tr><td></td><td><a href="index.php?module=bookmark&amp;action=modify&amp;id=<? echo $bookmark['id']; ?>"><button><? echo _html_safe(MODIFY); ?></button></a></td></tr>
<? } ?>
</table>
<? if(_module_id('category')) {
	_module('category', 'get', array('id' => $bookmark['id']));
} ?>
