		<div class="entry" onclick="entry_click(<? echo $explorer_id; ?>, <? echo $i; ?>, event)">
			<input class="hidden" type="checkbox" name="entry_<? echo $explorer_id; ?>_<? echo $i; ?>"/>
			<div class="icon"><? echo $link; ?><img src="<? echo $entry['icon']; ?>" alt=""/><? echo $link_end; ?></div>
			<div class="thumbnail"><img src="<? echo _html_safe_link($entry['thumbnail']); ?>" alt=""/></div>
			<div class="name"><? echo $link.$entry['name'].$link_end; ?></div>
<? if(isset($args['class'])) { $keys = array_keys($args['class']); foreach($keys as $k) { ?>
			<div class="<? echo _html_safe($k); ?>"><? echo $entry[$k]; ?></div>
<? } } ?>
		</div>
