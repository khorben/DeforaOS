<script type="text/javascript" src="modules/explorer/explorer.js"></script>
<form name="explorer_<? echo $explorer_id; ?>" class="explorer" action="index.php" method="post">
	<input type="hidden" name="module" value="explorer"/>
	<input type="hidden" name="action" value="apply"/>
	<input type="hidden" name="apply" value=""/>
<? if(strlen($module) && strlen($action)) { ?>
	<input type="hidden" name="link_module" value="<? echo _html_safe_link($module); ?>"/>
	<input type="hidden" name="link_action" value="<? echo _html_safe_link($action); ?>"/>
<? if(strlen($id)) { ?>
	<input type="hidden" name="link_id" value="<? echo _html_safe_link($id); ?>"/>
<? } } ?>
