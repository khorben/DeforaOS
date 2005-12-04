<script type="text/javascript" language="javascript" src="system/ajax.js"></script>
<h1><img src="modules/webmail/<? echo _html_safe($img); ?>.png" alt=""/> <? echo _html_safe(MESSAGE_LIST); ?></h1>
<div id="folders">
<h3>Folders <button onclick="ajax('index.php?module=webmail&action=folders&ajax=1')">Refresh</button></h3>
<? webmail_folders(array()); ?>
<h3><a href="index.php?module=webmail" style="display: block"><img src="modules/webmail/inbox.png" alt=""/> Mail</a></h3>
</div>
<div style="margin-left: 200px; padding-left: 4px">
