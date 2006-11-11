<div class="toolbar">
	<a href="index.php?module=project&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(HOMEPAGE); ?>"><img src="images/home.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(HOMEPAGE); ?></a>
<?php if(strlen($cvsroot)) { ?>
	&middot; <a href="index.php?module=project&amp;action=browse&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(BROWSE_SOURCE); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(BROWSE_SOURCE); ?></a>
	&middot; <a href="index.php?module=project&amp;action=timeline&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(TIMELINE); ?>"><img src="modules/project/icon.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(TIMELINE); ?></a>
<?php } ?>
	&middot; <a href="index.php?module=project&amp;action=bug_list&amp;project_id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(BUG_REPORTS); ?>"><img src="modules/project/bug.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(BUG_REPORTS); ?></a>
<?php if(_module_id('category') != 0 && _module_id('download') != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=download&amp;id=<?php echo _html_safe_link($id); ?>" title="<?php echo _html_safe_link(FILES); ?>"><img src="modules/project/files.png" alt="" style="height: 16px; width: 16px"/> <?php echo _html_safe_link(FILES); ?></a>
<?php }
if($admin != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=admin&amp;id=<?php echo _html_safe_link($id); ?>" title="Administration"><img src="modules/admin/icon.png" alt="" style="height: 16px; width: 16px"/> Administration</a>
<?php if($enabled != 0) { ?>
	&middot; <a href="index.php?module=project&amp;action=disable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1" title="<?php echo _html_safe_link(DISABLE); ?>"><img src="icons/16x16/disabled.png" alt=""/> <?php echo _html_safe_link(DISABLE); ?></a>
<?php } else { ?>
	&middot; <a href="index.php?module=project&amp;action=enable&amp;id=<?php echo _html_safe_link($id); ?>&amp;display=1" title="<?php echo _html_safe_link(ENABLE); ?>"><img src="icons/16x16/enabled.png" alt=""/> <?php echo _html_safe_link(ENABLE); ?></a>
<?php } } ?>
</div>
