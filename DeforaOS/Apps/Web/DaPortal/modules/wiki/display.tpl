<div class="wiki">
<?php if(isset($title)) { ?>
	<h1 class="title wiki"><?php echo _html_safe($title); ?></h1>
<?php } ?>
	<div id="wikicontent" class="content"><?php echo $wiki['content']; ?></div>
<?php global $user_id; if(isset($wiki['id']) && ($user_id != 0 || _config_get('wiki', 'anonymous') == TRUE)) { ?>
	<form>
	<a href="<?php echo _html_link('wiki', 'modify', $wiki['id'], $wiki['tag']); ?>"><button type="button" class="icon edit"><?php echo _html_safe(EDIT); ?></button></a>
	</form>
<?php } ?>
</div>
