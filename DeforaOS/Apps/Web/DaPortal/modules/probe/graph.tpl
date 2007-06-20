<div class="graph">
<h3><?php echo _html_safe($graph['title']); ?></h3>
<a href="<?php echo _html_link('probe', '', $graph['id'], '', 'type='._html_safe($graph['type'])); ?>" title="<?php echo _html_safe($graph['title']); ?>"><img src="<?php echo _html_safe($graph['img']); ?>" alt="<?php echo _html_safe($graph['name']); ?>"/></a>
</div>
