<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>Index of <?php echo html_safe($file); ?></title>
		<link type="text/css" rel="stylesheet" href="explorer.css"/>
		<script type="text/javascript" src="explorer.js"></script>
	</head>
	<body>
		<div class="explorer">
			<form name="explorer" action="explorer.php" method="post">
				<input type="hidden" name="file" value="<?php echo html_safe($file); ?>"/>
				<input type="hidden" name="sort" value="<?php echo html_safe($sort); ?>"/>
<?php if($reverse) { ?>
				<input type="hidden" name="reverse" value=""/>
<?php } ?>
				<input type="hidden" name="action" value=""/>
				<div class="toolbar">
					<img src="icons/16x16/back.png" alt="back" title="Back" onclick="history.back()"/>
					<a href="explorer.php?file=<?php echo html_safe_link(dirname($file)); ?>"><img src="icons/16x16/updir.png" alt="up one directory" title="Up one directory"/></a>
					<img src="icons/16x16/forward.png" alt="forward" title="Forward" onclick="history.forward()"/>
					<img src="icons/16x16/refresh.png" alt="refresh" title="Refresh" onclick="location.reload()"/>
					<div class="separator"></div>
<?php if($upload) { ?>
					<a href="newdir.php?folder=<?php echo html_safe_link($file); ?>"><img src="icons/16x16/newdir.png" alt="create directory" title="Create directory" onclick="return popup('newdir.php?folder=<?php echo html_safe_link($file); ?>')"/></a>
					<a href="upload.php?folder=<?php echo html_safe_link($file); ?>"><img src="icons/16x16/upload.png" alt="upload file" title="Upload file" onclick="return popup('upload.php?folder=<?php echo html_safe_link($file); ?>')"/></a>
					<img src="icons/16x16/delete.png" alt="delete" onclick="selection_delete()"/>
					<div class="separator"></div>
<?php } ?>
					<img src="icons/16x16/select_all.png" alt="select all" title="Select all" onclick="select_all()"/>
					<div class="separator"></div>
					<img src="icons/16x16/print.png" alt="print" title="Print" onclick="print()"/>
					<div class="separator"></div>
					<img src="icons/16x16/details.png" alt="details" title="Details" onclick="change_class('explorer_listing', 'listing_details')"/>
					<img src="icons/16x16/list.png" alt="list" title="List" onclick="change_class('explorer_listing', 'listing_list')"/>
					<img src="icons/16x16/thumbnails.png" alt="thumbnails" title="Thumbnails" onclick="change_class('explorer_listing', 'listing_thumbnails')"/>
				</div>
				<div id="explorer_listing" class="listing_details">
					<div class="header">
						<div class="icon"></div>
						<?php explorer_sort($file, 'name', $sort, $reverse); ?>
						<?php explorer_sort($file, 'owner', $sort, $reverse); ?>
						<?php explorer_sort($file, 'group', $sort, $reverse); ?>
						<?php explorer_sort($file, 'permissions', $sort, $reverse); ?>
						<?php explorer_sort($file, 'size', $sort, $reverse); ?>
						<?php explorer_sort($file, 'date', $sort, $reverse); ?>
					</div>
<?php explorer_folder($file, $dir, $sort, $reverse); ?>
					<script type="text/javascript"><!--
unselect_all();
					//--></script>
				</div>
			</form>
			<div style="clear: left">&nbsp;</div>
		</div>
	</body>
</html>
