	<div class="toolbar" style="text-align: center">
		<a href="<?php _html_link('webmail', '', '', '', 'folder='._html_safe($folder)); ?>">1</a>
<?php for($i = 1; $i * $mpp < $cnt; $i++) { ?>
		<span class="middot">&middot;</span>
		<a href="<?php <?php echo _html_link('webmail', FALSE, FALSE, FALSE, 'folder='._html_safe($folder).'&page='.($i+1)); ?>"><?php echo $i+1; ?></a>
<?php } ?>
	</div>
</div>
