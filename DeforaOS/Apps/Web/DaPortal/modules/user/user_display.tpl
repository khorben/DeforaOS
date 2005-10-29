<h1><img src="modules/user/user.png" alt=""/> <? echo _html_safe($user['username']); ?></h1>
<? _module('news', 'list', array('user_id' => $user['user_id'])); ?>
<? _module('comment', 'list', array('user_id' => $user['user_id'])); ?>
<? _module('project', 'list', array('user_id' => $user['user_id'])); ?>
<? _module('project', 'bug_list', array('user_id' => $user['user_id'])); ?>
