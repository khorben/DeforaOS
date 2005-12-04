<div class="toolbar">
	<a href="index.php?module=project&amp;id=<? echo _html_safe_link($id); ?>"><? echo _html_safe_link(HOMEPAGE); ?></a>
	· <a href="index.php?module=project&amp;action=browse&amp;id=<? echo _html_safe_link($id); ?>"><? echo _html_safe_link(BROWSE_SOURCE); ?></a>
	· <a href="index.php?module=project&amp;action=timeline&amp;id=<? echo _html_safe_link($id); ?>"><? echo _html_safe_link(TIMELINE); ?></a>
	· <a href="index.php?module=project&amp;action=bug_list&amp;project_id=<? echo _html_safe_link($id); ?>"><? echo _html_safe_link(BUG_REPORTS); ?></a>
	· <a href="index.php?module=project&amp;action=admin&amp;id=<? echo _html_safe_link($id); ?>">Administration</a>
</div>
