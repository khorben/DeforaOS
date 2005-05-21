		<div id="explorer_<? echo $explorer_id; ?>" class="listing_<? echo _html_safe($view); ?>">
			<div class="header">
				<div class="icon"></div>
				<div class="name">Name</div>
<? if(isset($args['class'])) { $keys = array_keys($args['class']); foreach($keys as $k) { ?>
				<div class="<? echo _html_safe($k); ?>"><? echo _html_safe($args['class'][$k]); ?></div>
<? } } ?>
			</div>
