<div class="toolbar">
	<a href="index.php?module=project&amp;id=<? echo _html_safe_link($id); ?>"><img src="modules/user/home.png" alt="" style="height: 16px; width: 16px"/> <? echo _html_safe_link(HOMEPAGE); ?></a>
<? if(strlen($cvsroot)) { ?>
	· <a href="index.php?module=project&amp;action=browse&amp;id=<? echo _html_safe_link($id); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <? echo _html_safe_link(BROWSE_SOURCE); ?></a>
	· <a href="index.php?module=project&amp;action=timeline&amp;id=<? echo _html_safe_link($id); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <? echo _html_safe_link(TIMELINE); ?></a>
<? } ?>
	· <a href="index.php?module=project&amp;action=bug_list&amp;project_id=<? echo _html_safe_link($id); ?>"><img src="modules/project/bug.png" alt="" style="height: 16px; width: 16px"/> <? echo _html_safe_link(BUG_REPORTS); ?></a>
<? if($admin != 0) { ?>
	· <a href="index.php?module=project&amp;action=admin&amp;id=<? echo _html_safe_link($id); ?>"><img src="modules/admin/icon.png" alt="" style="height: 16px; width: 16px"/> Administration</a>
<? if($enabled != 0) { ?>
	· <a href="index.php?module=project&amp;action=disable&amp;id=<? echo _html_safe_link($id); ?>&amp;display=1"><img src="icons/16x16/disabled.png" alt=""/> Disable</a>
<? } else { ?>
	· <a href="index.php?module=project&amp;action=enable&amp;id=<? echo _html_safe_link($id); ?>&amp;display=1"><img src="icons/16x16/enabled.png" alt=""/> Enable</a>
<? } } ?>
</div>
