<h1><img src="modules/project/icon.png" alt=""/> <? echo _html_safe($title); ?></h1>
<? if(strlen($project['title'])) { ?><div class="title"><? echo _html_safe($project['title']); ?></div><? } ?>
<? if(strlen($project['description'])) { ?><div class="headline"><? echo _html_pre($project['description']); ?></div><? } ?>
