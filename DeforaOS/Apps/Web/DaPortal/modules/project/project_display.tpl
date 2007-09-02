<h1 class="title project"><?php echo _html_safe($title); ?></h1>
<?php if(strlen($project['synopsis'])) { ?><div class="title"><?php echo _html_safe($project['synopsis']); ?></div><?php } ?>
<?php if(strlen($project['description'])) { ?><div class="headline"><?php echo _html_pre($project['description']); ?></div><?php } ?>
