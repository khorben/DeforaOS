	<div class="toolbar">
<? if(isset($args['toolbar'])) { ?>
<? foreach($args['toolbar'] as $t) { ?>
		<a href="<? echo _html_safe_link($t['link']); ?>"><img src="<? echo _html_safe_link($t['icon']); ?>" alt="" title="<? echo _html_safe($t['title']); ?>"/></a>
<? } ?>
		<div class="separator"></div>
<? } ?>
		<img src="modules/explorer/select_all.png" alt="select all" title="<? echo _html_safe(SELECT_ALL); ?>" onclick="select_all(<? echo $explorer_id; ?>)"/>
		<div class="separator"></div>
		<img src="modules/explorer/details.png" alt="details" title="<? echo _html_safe(LISTING_DETAILS); ?>" onclick="change_class('explorer_<? echo $explorer_id; ?>', 'listing_details')"/>
		<img src="modules/explorer/list.png" alt="list" title="<? echo _html_safe(LISTING_LIST); ?>" onclick="change_class('explorer_<? echo $explorer_id; ?>', 'listing_list')"/>
		<img src="modules/explorer/thumbnails.png" alt="thumbnails" title="<? echo _html_safe(LISTING_THUMBNAILS); ?>" onclick="change_class('explorer_<? echo $explorer_id; ?>', 'listing_thumbnails')"/>
	</div>
