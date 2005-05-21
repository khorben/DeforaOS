<h1><img src="modules/user/icon.png" alt=""/> <? echo _html_safe($user['username']); ?></h1>
<? _module('news', 'list', array('user_id' => $user['user_id'])); ?>
<? _module('project', 'list', array('user_id' => $user['user_id'])); ?>
