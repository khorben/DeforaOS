<table class="toolbar" width="100%">
<? if(strlen($from)) { ?>
	<tr><td class="field">From:</td><td><? echo _html_safe($from); ?></td></tr>
<? } ?>
<? if(strlen($subject)) { ?>
	<tr><td class="field">Subject:</td><td><? echo _html_safe($subject); ?></td></tr>
<? } ?>
<? if(strlen($to)) { ?>
	<tr><td class="field">To:</td><td><? echo _html_safe($to); ?></td></tr>
<? } ?>
<? if(strlen($cc)) { ?>
	<tr><td class="field">Carbon copy:</td><td><? echo _html_safe($cc); ?></td></tr>
<? } ?>
</table>
<pre>
<? echo _html_safe($body); ?>
</pre>
