<h1><img src="modules/project/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<? if(strlen($project['title'])) { ?><div class="title"><? echo _html_safe($project['title']); ?></div><? } ?>
<? if(strlen($project['description'])) { ?><div class="headline"><? echo _html_pre($project['description']); ?></div><? } ?>
<table>
	<tr><td class="field"><? echo _html_safe(ADMINISTRATOR); ?>:</td><td><a href="index.php?module=user&amp;id=<? echo _html_safe_link($project['user_id']); ?>"><? echo _html_safe($project['username']); ?></a></td></tr>
	<tr><td class="field"><? echo _html_safe(MEMBERS); ?>:</td><td><?
foreach($project['members'] as $member) { ?>
<a href="index.php?module=user&amp;id=<? echo _html_safe_link($member['id']); ?>"><? echo _html_safe($member['name']); ?></a><br/>
<? } ?></td></tr>
</table>
