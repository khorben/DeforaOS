<h1><img src="modules/user/icon.png" alt=""/> <? echo _html_safe($user_name); ?>'s page</h1>
<? global $user_id; require_once('system/user.php'); if(_user_admin($user_id)) { ?>
<p>You are an <a href="index.php?module=admin">administrator</a>.</p>
<? } ?>
<p><a href="index.php?module=user&action=logout">Logout</a></p>
<? _module('news', 'list', array('user_id' => $user_id)); ?>
<? _module('project', 'list', array('user_id' => $user_id)); ?>
