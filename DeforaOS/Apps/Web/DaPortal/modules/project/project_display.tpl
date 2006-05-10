<h1><img src="modules/project/icon.png" alt=""/> <?php echo _html_safe($title); ?></h1>
<?php if(strlen($project['title'])) { ?><div class="title"><?php echo _html_safe($project['title']); ?></div><?php } ?>
<?php if(strlen($project['description'])) { ?><div class="headline"><?php echo _html_pre($project['description']); ?></div><?php } ?>
