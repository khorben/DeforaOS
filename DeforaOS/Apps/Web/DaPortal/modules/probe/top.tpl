<h1 class="title probe"><?php echo _html_safe($title); ?></h1>
<?php if(isset($toolbar)) { ?>
<div class="toolbar">
<?php $middot = ''; foreach($toolbar as $t) { echo $middot; ?>
<a href="index.php?module=probe&amp;type=<?php echo _html_safe_link($t['type']); if(isset($id)) echo '&amp;id='._html_safe_link($id); if(isset($time)) echo '&amp;time='._html_safe_link($time); ?>" title="<?php echo _html_safe($t['title']); ?>"><?php echo _html_safe($t['name']); ?></a>
<?php $middot = '	&middot; '; } ?>
</div>
<?php } if(isset($action)) { ?>
<div style="padding: 4px">
	View last: <form action="index.php" method="get" style="display: inline">
		<input type="hidden" name="module" value="probe"/>
		<input type="hidden" name="action" value="<?php echo _html_safe($action); ?>"/>
<?php if(isset($id)) { ?>
		<input type="hidden" name="id" value="<?php echo $id; ?>"/>
<?php } ?>
<?php if(isset($type)) { ?>
		<input type="hidden" name="type" value="<?php echo $type; ?>"/>
<?php } ?>
		<select name="time" onchange="submit()">
			<option value="all">all</option>
			<option value="hour"<?php if(isset($time) && $time == 'hour') { ?> selected="selected"<?php } ?>>hour</option>
			<option value="day"<?php if(isset($time) && $time == 'day') { ?> selected="selected"<?php } ?>>day</option>
			<option value="week"<?php if(isset($time) && $time == 'week') { ?> selected="selected"<?php } ?>>week</option>
		</select>
		<input id="probe_view" type="submit" value="View"/>
		<script type="text/javascript"><!--
document.getElementById('probe_view').style.display='none';
//--></script>
	</form>
</div>
<?php } ?>
<div class="probe">
