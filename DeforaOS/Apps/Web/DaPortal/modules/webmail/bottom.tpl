<div>
<a href="index.php?module=webmail&amp;folder=<? echo _html_safe($folder); ?>">1</a>
<? for($i = 1; $i * $mpp < $cnt; $i++) { ?>
 · <a href="index.php?module=webmail&amp;folder=<? echo _html_safe($folder); ?>&amp;page=<? echo $i+1; ?>"><? echo $i+1; ?></a>
<? } ?>
</div>
</div>
