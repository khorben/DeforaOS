<div class="toolbar" style="text-align: center">
<a href="index.php?module=webmail&amp;folder=<?php echo _html_safe($folder); ?>">1</a>
<?php for($i = 1; $i * $mpp < $cnt; $i++) { ?>
 &middot; <a href="index.php?module=webmail&amp;folder=<?php echo _html_safe($folder); ?>&amp;page=<?php echo $i+1; ?>"><?php echo $i+1; ?></a>
<?php } ?>
</div>
</div>
