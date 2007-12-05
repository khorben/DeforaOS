<h1 class="title bug"><?php echo _html_safe(BUG_REPORT); ?></h1>
<?php $text = _html_safe(YOUR_BUG_IS_SUBMITTED);
if(!$enable) $text.=' '._html_safe(AND_AWAITS_MODERATION);
$text.=". "._html_safe(THANK_YOU).'!';
_info($text, 1); ?>
