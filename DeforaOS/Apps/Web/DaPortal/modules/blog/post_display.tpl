<?php if(isset($title)) { ?>
<h1 class="title blog"><?php echo _html_safe($title); ?></h1>
<?php } ?>
<?php if(isset($description)) { ?>
<div class="blog description"><?php echo _html_safe($description); ?></div>
<?php } ?>
<div class="blog entry">
<?php if(isset($post['title'])) { ?>
	<div class="title"><span><span><span><?php if(!$long) { ?><a href="<?php echo _html_link('blog', FALSE, $post['id'], $post['tag']); ?>"><?php } ?><?php echo _html_safe($post['title']); ?><?php if(!$long) { ?></a><?php } ?></span></span></span></div>
<?php } ?>
	<div class="author"><?php echo _html_safe(BLOG_POST._BY_); ?> <a href="<?php echo _html_link('user', FALSE, $post['user_id'], $post['username']); ?>"><?php echo _html_safe($post['username']); ?></a></div>
	<div class="date"><?php echo _html_safe(BLOG_ON); ?> <?php echo _html_safe($post['date']); ?></div>
<?php if(isset($post['id']) && _module_id('category')) _module('category', 'get', array('id' => $post['id'])); ?>
	<div class="content"><?php echo _html_pre($post['content']); ?></div>
	<div class="status">
<?php if(isset($post['preview'])) { ?>
	<?php echo _html_safe(BLOG_PREVIEW); ?>
<?php } else { ?>
		<a href="<?php echo _html_link('blog', FALSE, $post['id'], $post['tag']); ?>"><div class="icon read"></div> <?php echo _html_safe(READ); ?></a>
<?php if(_module_id('comment')) { ?>
		(<?php echo _html_safe(_module('comment', 'count', array('id' => $post['id'])).' '.COMMENT_S); ?>)
		<span class="middot">&middot;</span>
		<a href="<?php echo _html_link('blog', 'reply', $post['id']); ?>#edit"><div class="icon reply"></div> <?php echo _html_safe(REPLY); ?></a>
<?php } ?>
<?php global $user_id; require_once('./system/user.php');
if(isset($post['id']) && (_user_admin($user_id) || $user_id == $post['user_id'])) { ?>
		<span class="middot">&middot;</span>
		<a href="<?php echo _html_link('blog', 'update', $post['id']); ?>"><div class="icon edit"></div> <?php echo _html_safe(EDIT); ?></a>
<?php } } ?>
	</div>
</div>
