<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>Index of <? echo html_safe($folder); ?></title>
		<link type="text/css" rel="stylesheet" href="explorer.css"/>
		<script type="text/javascript" src="explorer.js"></script>
	</head>
	<body>
		<div class="explorer">
			<form name="explorer_form" action="explorer.php" method="post">
				<div class="toolbar">
					<img src="icons/16x16/select_all.png" alt="select all" onclick="select_all()"/>
					<img src="icons/16x16/refresh.png" alt="refresh" onclick="location.reload()"/>
					<img src="icons/16x16/print.png" alt="print" onclick="print()"/>
					<div class="separator"></div>
					<img src="icons/16x16/details.png" alt="details" onclick="change_class('explorer_listing', 'listing_details')"/>
					<img src="icons/16x16/list.png" alt="list" onclick="change_class('explorer_listing', 'listing_list')"/>
					<img src="icons/16x16/thumbnails.png" alt="thumbnails" onclick="change_class('explorer_listing', 'listing_thumbnails')"/>
				</div>
				<div id="explorer_listing" class="listing_details">
					<div class="header">
						<div class="icon"></div>
						<? explorer_sort($folder, 'name', $sort, $reverse); ?>
						<? explorer_sort($folder, 'owner', $sort, $reverse); ?>
						<? explorer_sort($folder, 'group', $sort, $reverse); ?>
						<? explorer_sort($folder, 'size', $sort, $reverse); ?>
						<? explorer_sort($folder, 'date', $sort, $reverse); ?>
					</div>
<? explorer_folder($folder, $sort, $reverse); ?>
					<script type="text/javascript">
<!--
unselect_all();
//-->					</script>
				</div>
			</form>
			<div style="clear: left">&nbsp;</div>
		</div>
	</body>
</html>
