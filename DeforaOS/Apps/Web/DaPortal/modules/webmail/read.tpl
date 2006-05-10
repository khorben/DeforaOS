<table class="toolbar" width="100%">
<?php if(strlen($from)) { ?>
	<tr><td class="field">From:</td><td><?php echo _html_safe($from); ?></td></tr>
<?php } ?>
<?php if(strlen($subject)) { ?>
	<tr><td class="field">Subject:</td><td><?php echo _html_safe($subject); ?></td></tr>
<?php } ?>
<?php if(strlen($to)) { ?>
	<tr><td class="field">To:</td><td><?php echo _html_safe($to); ?></td></tr>
<?php } ?>
<?php if(strlen($cc)) { ?>
	<tr><td class="field">Carbon copy:</td><td><?php echo _html_safe($cc); ?></td></tr>
<?php } ?>
</table>
<pre>
<?php echo _html_safe($body); ?>
</pre>
