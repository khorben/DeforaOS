<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



//check url
if(preg_match('/\/index.php$/', $_SERVER['SCRIPT_NAME']) != 1)
	exit(header('Location: '.dirname($_SERVER['SCRIPT_NAME'])));


//lang
$text = array();
$text['BLOG'] = 'Blog';
$text['BLOG_ADMINISTRATION'] = 'Blog administration';
$text['BLOG_BY'] = 'Blog by';
$text['BLOG_LIST'] = 'Blog list';
$text['BLOG_ON'] = 'on';
$text['BLOG_PLANET'] = 'Planet';
$text['BLOG_POST'] = 'Blog post';
$text['BLOG_POSTS'] = 'Blog posts';
$text['BLOG_PREVIEW'] = 'Blog post preview';
$text['BLOGS_REGISTERED'] = 'Blogs registered';
$text['COMMENT_S'] = 'comment(s)';
$text['COMMENTS'] = 'Comments';
$text['NEW_BLOG_POST'] = 'New blog post';
$text['THEME'] = 'Theme';
global $lang;
if($lang == 'fr')
{
	$text['BLOG_POSTS'] = 'Billets';
	$text['NEW_BLOG_POST'] = 'Nouveau billet';
	$text['THEME'] = 'Th�me';
}
_lang($text);


//BlogModule
class BlogModule extends Module
{
	//public
	//methods
	//useful
	//BlogModule::call
	public function call(&$engine, $request, $internal = 0)
	{
		$args = $request->getParameters();
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'disable':
			case 'display':
			case 'enable':
			case 'headline':
			case 'insert':
			case 'planet':
			case 'rss':
			case 'system':
			case 'update':
				return $this->$action($args);
			case 'list':
				return $this->_list($args);
			default:
				return $this->_default($args);
		}
	}


//private
//accessors
//BlogModule::getDescription
private function getDescription($id)
{
	$sql = 'SELECT blog.content AS description'
		.' FROM daportal_content blog, daportal_blog_user'
		.', daportal_content post, daportal_blog_content'
		.' WHERE blog.content_id=daportal_blog_user.blog_user_id'
		.' AND blog.user_id=post.user_id'
		.' AND post.content_id=daportal_blog_content.blog_content_id'
		." AND daportal_blog_content.blog_content_id='$id'";
	if(($res = _sql_single($sql)) != FALSE)
		return $res;
	return '';
}


//BlogModule::getDescriptionUser
private function getDescriptionUser($user_id)
{
	$sql = 'SELECT content'
		.' FROM daportal_content, daportal_blog_user'
		.' WHERE daportal_content.content_id'
		.'=daportal_blog_user.blog_user_id'
		." AND daportal_content.user_id='$user_id'";
	if(($res = _sql_single($sql)) != FALSE)
		return $res;
	return '';
}


//BlogModule::getTitle
private function getTitle($id)
{
	$sql = 'SELECT blog.title AS title'
		.' FROM daportal_content blog, daportal_blog_user'
		.', daportal_content post, daportal_blog_content'
		.' WHERE blog.content_id=daportal_blog_user.blog_user_id'
		.' AND blog.user_id=post.user_id'
		.' AND post.content_id=daportal_blog_content.blog_content_id'
		." AND daportal_blog_content.blog_content_id='$id'";
	if(($res = _sql_single($sql)) != FALSE)
		return $res;
	require_once('./system/content.php');
	if(($content = _content_select($id, TRUE)) != FALSE)
		return BLOG_BY.' '.$content['username'];
	return BLOG;
}


//BlogModule::getTitleUser
private function getTitleUser($user_id)
{
	$sql = 'SELECT title'
		.' FROM daportal_content, daportal_blog_user'
		.' WHERE daportal_content.content_id'
		.'=daportal_blog_user.blog_user_id'
		." AND daportal_content.user_id='$user_id'";
	if(($res = _sql_single($sql)) != FALSE)
		return $res;
	require_once('./system/user.php');
	return BLOG_BY.' '._user_name($user_id);
}


//useful
//BlogModule::_insert
private function _insert($post)
{
	global $user_id;

	if($user_id == 0)
		return FALSE;
	require_once('./system/content.php');
	if(($id = _content_insert($post['title'], $post['content'], 0))
			== FALSE)
	{
		_error('Could not insert blog post');
		return FALSE;
	}
	$comment = (isset($post['comment']) && $post['comment'] == 1) ? 1 : 0;
	if(_sql_query('INSERT INTO daportal_blog_content (blog_content_id, '
					."comment) VALUES ('$id', '$comment')")
				== FALSE)
	{
		_content_delete($id);
		_error('Could not insert blog post');
		return FALSE;
	}
	return $id;
}


//BlogModule::list_blogs
private function listBlogs($args)
{
	print('<h1 class="title blog">'._html_safe(BLOG_LIST)."</h1>\n");
	$sql = 'SELECT title AS name, daportal_user.user_id AS user_id'
		.', username'
		.' FROM daportal_content, daportal_blog_user, daportal_user'
		.' WHERE daportal_content.content_id'
		.' =daportal_blog_user.blog_user_id'
		.' AND daportal_content.user_id=daportal_user.user_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_user.enabled='1'";
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list blogs');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['icon'] = 'icons/16x16/blog.png';
		$res[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$res[$i]['name'] = '<a href="'._html_link('blog', FALSE, FALSE,
			FALSE, array('user' => $res[$i]['username']))
			.'">'._html_safe($res[$i]['name']).'</a>';
		$res[$i]['username'] = '<a href="'._html_link('user', '',
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
	}
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('username' => AUTHOR),
				'module' => 'blog', 'toolbar' => FALSE,
				'view' => 'details'));
}


//public
//BlogModule::admin
protected function admin($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	print('<h1 class="title blog">'._html_safe(BLOG_ADMINISTRATION)
			."</h1>\n");
	print('<h2 class="title blog">'._html_safe(BLOGS_REGISTERED)."</h2>\n");
	$module = _module_id('blog');
	$sql = 'SELECT content_id AS id, daportal_content.enabled AS enabled'
		.', title AS name, daportal_user.user_id AS user_id, username'
		.', theme FROM daportal_content, daportal_user'
		.', daportal_blog_user WHERE daportal_user.user_id'
		.'=daportal_content.user_id AND daportal_content.content_id'
		.'=daportal_blog_user.blog_user_id';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Unable to list blogs');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'blog';
		$res[$i]['apply_module'] = 'blog';
		$res[$i]['action'] = 'update';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'icons/16x16/blog.png';
		$res[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$res[$i]['name'] = _html_safe($res[$i]['name']);
		$res[$i]['username'] = '<a href="'._html_link('user', '',
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['theme'] = _html_safe($res[$i]['theme']);
	}
	$toolbar = array();
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'theme' => THEME,
					'username' => AUTHOR),
				'module' => 'blog', 'action' => 'admin',
				'toolbar' => $toolbar, 'view' => 'details'));
	print('<h2 class="title blog">'._html_safe(BLOG_POSTS)."</h2>\n");
	$order = 'DESC';
	$sort = 'timestamp';
	if(isset($args['sort']))
	{
		$order = 'ASC';
		switch($args['sort'])
		{
			case 'username':$sort = 'username';	break;
			case 'enabled':	$sort = 'daportal_content.enabled';
								break;
			case 'name':	$sort = 'title';	break;
			default:	$order = 'DESC';	break;
		}
	}
	$sql = 'SELECT content_id AS id, timestamp'
		.', daportal_content.enabled AS enabled, title AS name, content'
		.', daportal_content.user_id AS user_id, username, comment'
		.' FROM daportal_content, daportal_user, daportal_blog_content'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		.' AND daportal_content.content_id'
		.'=daportal_blog_content.blog_content_id'
		." AND module_id='$module' ORDER BY ".$sort.' '.$order;
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Unable to list posts');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'blog';
		$res[$i]['apply_module'] = 'blog';
		$res[$i]['action'] = 'update';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['icon'] = 'icons/16x16/blog.png';
		$res[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$res[$i]['name'] = _html_safe($res[$i]['name']);
		$res[$i]['username'] = '<a href="'._html_link('user', '',
			$res[$i]['user_id'], $res[$i]['username']).'">'
				._html_safe($res[$i]['username']).'</a>';
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['comment'] = $res[$i]['comment'] == SQL_TRUE ?
			'enabled' : 'disabled';
		$res[$i]['comment'] = '<img src="icons/16x16/'
				.$res[$i]['comment'].'.png" alt="'
				.$res[$i]['comment'].'" title="'
				.($res[$i]['comment'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
		$res[$i]['date'] = _html_safe(strftime('%d/%m/%y %H:%M',
					strtotime(substr($res[$i]['timestamp'],
							0, 19))));
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_BLOG_POST, 'class' => 'new',
			'link' => _module_link('blog', 'insert'));
	$toolbar[] = array();
	$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
			'action' => 'disable');
	$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
			'action' => 'enable');
	$toolbar[] = array();
	$toolbar[] = array('title' => DELETE, 'class' => 'delete',
			'action' => 'delete', 'confirm' => 'delete');
	_module('explorer', 'browse_trusted', array('entries' => $res,
				'class' => array('enabled' => ENABLED,
					'comment' => COMMENTS,
					'username' => AUTHOR, 'date' => DATE),
				'module' => 'blog', 'action' => 'admin',
				'sort' => isset($args['sort']) ? $args['sort']
						: 'date',
				'toolbar' => $toolbar, 'view' => 'details'));
}


//BlogModule::_default
protected function _default($args)
{
	if(!isset($args['id']))
		return $this->planet($args);
	$this->display($args);
}


//BlogModule::delete
//FIXME allow to delete one's own content
protected function delete($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_delete($args['id']))
		return _error('Could not delete post');
}


//BlogModule::disable
//FIXME allow to disable one's own content
protected function disable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_disable($args['id']))
		return _error('Could not disable post');
}


//BlogModule::display
protected function display($args)
{
	/* FIXME make a difference between blog descriptions and posts */
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($post = _content_select($args['id'], 1)) == FALSE)
	{
		_error(INVALID_ARGUMENT);
		return FALSE;
	}
	$title = $this->getTitle($args['id']);
	$description = $this->getDescription($args['id']);
	$long = 1;
	$post['date'] = _sql_date($post['timestamp']);
	include('./modules/blog/post_display.tpl');
	return TRUE;
}


//BlogModule::enable
//FIXME allow to enable one's own content
protected function enable($args)
{
	global $user_id;

	require_once('./system/user.php');
	if(!_user_admin($user_id))
		return _error(PERMISSION_DENIED);
	require_once('./system/content.php');
	if(!_content_enable($args['id']))
		return _error('Could not enable post');
}


//BlogModule::headline
protected function headline($args)
{
	$page = 1;
	$npp = 10;
	if(isset($args['npp']) && is_numeric($args['npp']))
		$npp = $args['npp'];
	//FIXME use the daportal_blog_content table too
	$posts = _sql_array('SELECT content_id AS id, title, name AS module'
			.' FROM daportal_content, daportal_module'
			.' WHERE daportal_content.module_id'
			.'=daportal_module.module_id'
			." AND daportal_module.name='blog'"
			." AND daportal_content.enabled='1'"
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page-1) * $npp, $npp)));
	if(!is_array($posts))
		return _error('Could not list posts');
	for($i = 0, $cnt = count($posts); $i < $cnt; $i++)
	{
		$posts[$i]['action'] = 'default';
		$posts[$i]['icon'] = 'icons/16x16/blog.png';
		$posts[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$posts[$i]['name'] = $posts[$i]['title'];
		$posts[$i]['tag'] = $posts[$i]['title'];
	}
	_module('explorer', 'browse', array('toolbar' => 0, 'view' => 'details',
				'header' => 0, 'entries' => $posts));
}


//BlogModule::insert
protected function insert($args)
{
	global $error, $user_id, $user_name;

	if(isset($error) && strlen($error))
		return _error($error);
	$title = NEW_BLOG_POST;
	if(isset($args['preview']))
	{
		$long = 1;
		$title = NEW_BLOG_POST;
		$post = array('user_id' => $user_id,
				'username' => $user_name,
				'title' => stripslashes($args['title']),
				'content' => stripslashes($args['content']),
				'date' => strftime(DATE_FORMAT),
				'preview' => 1);
		include('./modules/blog/post_display.tpl');
		unset($title);
	}
	include('./modules/blog/post_update.tpl');
}


//BlogModule::_list
protected function _list($args)
{
	global $user_id;

	if(!isset($args['user_id']) && !isset($args['user']))
		return $this->listBlogs($args);
	$title = BLOG_POSTS;
	$and = '';
	$myself = 0;
	require_once('./system/user.php');
	if(isset($args['user_id'])
			&& ($username = _user_name($args['user_id'])) != FALSE)
	{
		$title = BLOG_POSTS._BY_.' '.$username;
		$and = " AND daportal_user.user_id='".$args['user_id']."'";
		if($user_id == $args['user_id'])
			$myself = 1;
		else
			$and.=" AND daportal_content.enabled='1'";
		$paging = 'user_id='._html_safe($args['user_id']).'&amp;';
	}
	print('<h1 class="title blog">'._html_safe($title)."</h1>\n");
	unset($title); //XXX hoping this doesn't affect the global variable
	$sql = 'SELECT content_id AS id, timestamp AS date, title, content'
		.', daportal_content.enabled AS enabled'
		.', daportal_content.user_id AS user_id, username'
		.', name AS module'
		.' FROM daportal_module, daportal_content, daportal_user'
		.', daportal_blog_content'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		.' AND daportal_content.content_id'
		.'=daportal_blog_content.blog_content_id'
		." AND daportal_module.name='blog'".$and
		.' AND daportal_module.module_id=daportal_content.module_id'
		.' ORDER BY timestamp DESC';
	$res = _sql_array($sql);
	if(!is_array($res))
		return _error('Could not list blog posts');
	for($i = 0, $cnt = count($res); $i < $cnt; $i++)
	{
		$res[$i]['module'] = 'blog';
		$res[$i]['action'] = 'display';
		$res[$i]['icon'] = 'icons/16x16/blog.png';
		$res[$i]['thumbnail'] = 'icons/48x48/blog.png';
		$res[$i]['name'] = _html_safe($res[$i]['title']);
		$res[$i]['tag'] = _html_safe($res[$i]['title']);
		$res[$i]['date'] = strftime('%d/%m/%Y %H:%M', strtotime(substr(
						$res[$i]['date'], 0, 19)));
		$res[$i]['date'] = _html_safe($res[$i]['date']);
		if(!$myself)
			continue;
		$res[$i]['apply_module'] = 'blog';
		$res[$i]['apply_id'] = $res[$i]['id'];
		$res[$i]['enabled'] = $res[$i]['enabled'] == SQL_TRUE
			? 'enabled' : 'disabled';
		$res[$i]['enabled'] = '<img src="icons/16x16/'
				.$res[$i]['enabled'].'.png" alt="'
				.$res[$i]['enabled'].'" title="'
				.($res[$i]['enabled'] == 'enabled'
						? ENABLED : DISABLED).'"/>';
	}
	$toolbar = array();
	$toolbar[] = array('title' => NEW_BLOG_POST, 'class' => 'new',
			'link' => _module_link('blog', 'insert'));
	if($myself)
	{
		$toolbar[] = array('title' => DISABLE, 'class' => 'disabled',
				'action' => 'disable');
		$toolbar[] = array('title' => ENABLE, 'class' => 'enabled',
				'action' => 'enable');
	}
	$class = $myself ? array('enabled' => ENABLED, 'date' => DATE)
		: array('date' => DATE);
	//FIXME the redirection is incomplete (need to specify the user_id)
	_module('explorer', 'browse_trusted', array('entries' => $res,
			'module' => 'blog', 'action' => 'list',
			'class' => $class, 'view' => 'details', 
			'toolbar' => $toolbar));
}


//BlogModule::planet
//FIXME code duplication with blog_list
protected function planet($args)
{
	$title = BLOG_PLANET;
	$and = '';
	$paging = '';
	require_once('./system/user.php');
	if(!isset($args['user_id']) && isset($args['user']))
		$args['user_id'] = _user_id($args['user']);
	if(isset($args['user_id']) && $args['user_id'] != FALSE
			&& ($username = _user_name($args['user_id'])) != FALSE)
	{
		$title = $this->getTitleUser($args['user_id']);
		$description = $this->getDescriptionUser($args['user_id']);
		$and = " AND daportal_user.user_id='".$args['user_id']."'";
		$paging = 'user_id='._html_safe($args['user_id']).'&';
	}
	print('<h1 class="title blog">'._html_safe($title)."</h1>\n");
	unset($title); //XXX hoping this doesn't affect the global variable
	$sql = ' FROM daportal_module, daportal_content, daportal_user'
		.', daportal_blog_content'
		.' WHERE daportal_user.user_id=daportal_content.user_id'
		.' AND daportal_content.content_id'
		.'=daportal_blog_content.blog_content_id'
		." AND daportal_content.enabled='1'"
		." AND daportal_module.name='blog'".$and
		.' AND daportal_module.module_id=daportal_content.module_id';
	$npp = 10;
	$page = isset($args['page']) ? $args['page'] : 1;
	if(($cnt = _sql_single('SELECT COUNT(*)'.$sql)) == 0)
		$cnt = 1;
	$pages = ceil($cnt / $npp);
	$page = min($page, $pages);
	$res = _sql_array('SELECT content_id AS id, timestamp, title, content'
			.', daportal_content.enabled AS enabled'
			.', daportal_content.user_id AS user_id, username'.$sql
			.' ORDER BY timestamp DESC '
			.(_sql_offset(($page - 1) * $npp, $npp)));
	if(!is_array($res))
		return _error('Unable to list posts');
	$long = 0;
	require_once('./system/content.php');
	foreach($res as $post)
	{
		$post['tag'] = $post['title'];
		_content_select_lang($post['id'], $post['title'],
				$post['content']);
		$post['date'] = _sql_date($post['timestamp']);
		include('./modules/blog/post_display.tpl');
		unset($description);
	}
	_html_paging(_html_link('blog', 'planet', FALSE, FALSE,
				$paging.'page='), $page, $pages);
}


//BlogModule::rss
protected function rss($args)
{
	global $title;

	if(($module_id = _module_id('blog')) == FALSE)
		return;
	$and = '';
	if(isset($args['user_id']) || isset($args['user']))
	{
		require_once('./system/user.php');
		if(isset($args['user']))
			$args['user_id'] = _user_id($args['user']);
		else
			$args['user'] = _user_name($args['user_id']);
		$and = " AND daportal_user.user_id='".$args['user_id']."'";
		$link = _module_link_full('blog', FALSE, FALSE,
				array('user' => $args['user']));
		$link = _module_link_full('blog', 'rss', FALSE, FALSE,
				array('user' => $args['user']));
		$atomlink = _module_link_full('blog', 'rss', FALSE, FALSE,
				array('user' => $args['user']));
		//FIXME get the description instead
		$content = $title.' - '.BLOG_BY.' '.$args['user'];
		$title = $this->getTitleUser($args['user_id']);
	}
	else
	{
		$link = _module_link_full('blog');
		$atomlink = _module_link_full('blog', 'rss');
		$title = $title.' - '.BLOG_PLANET;
		$content = $title;
	}
	require_once('./system/html.php');
	$res = _sql_array('SELECT content_id AS id, timestamp AS date, title'
			.', content, username AS author, email'
			.' FROM daportal_content, daportal_user'
			.', daportal_blog_content'
			.' WHERE daportal_content.user_id'
			.'=daportal_user.user_id'
			.' AND daportal_content.content_id'
			.'=daportal_blog_content.blog_content_id'
			." AND module_id='$module_id'"
			." AND daportal_content.enabled='1'".$and
			.' ORDER BY timestamp DESC '._sql_offset(0, 10));
	if(is_array($res))
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$res[$i]['author'] = $res[$i]['email']
				.' ('.$res[$i]['author'].')';
			$res[$i]['date'] = date('D, j M Y H:i:s O', strtotime(
						substr($res[$i]['date'], 0,
						19)));
			$res[$i]['link'] = _html_link_full('blog', FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$res[$i]['content'] = _html_pre($res[$i]['content']);
		}
	require_once('./system/rss.php');
	_rss($title, $link, $atomlink, $content, $res);
}


//BlogModule::system
protected function system($args)
{
	global $html, $error;

	if($_SERVER['REQUEST_METHOD'] == 'GET')
	{
		if(isset($args['action']) && $args['action'] == 'rss')
		{
			$html = 0;
			header('Content-Type: text/xml');
		}
	}
	else if($_SERVER['REQUEST_METHOD'] == 'POST')
		switch($args['action'])
		{
			case 'insert':
				$error = $this->_system_insert($args);
				return;
			case 'update':
				$error = $this->_system_update($args);
				return;
		}
}

private function _system_insert($args)
{
	global $user_id, $user_name;

	if($user_id == 0)
		return PERMISSION_DENIED;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	$post = array('user_id' => $user_id, 'username' => $user_name,
			'title' => $args['title'],
			'content' => $args['content']);
	if(($post['id'] = $this->_insert($post)) == FALSE)
		return 'Could not insert blog post';
	header('Location: '._module_link('blog', FALSE, $post['id']));
	exit(0);
}

private function _system_update($args)
{
	if(!isset($args['id']) || !isset($args['title'])
			|| !isset($args['content'])
			|| !isset($args['timestamp']))
		return INVALID_ARGUMENT;
	if(isset($args['preview']) || !isset($args['send']))
		return;
	require_once('./system/content.php');
	if(!_content_user_update($args['id'], $args['title'], $args['content'],
				$args['timestamp']))
		return PERMISSION_DENIED;
	header('Location: '._module_link('blog', FALSE, $args['id']));
	exit(0);
}


//BlogModule::update
protected function update($args)
{
	global $error, $user_id;

	if(isset($error) && strlen($error))
		_error($error);
	if(!isset($args['id']))
		return _error(INVALID_ARGUMENT);
	require_once('./system/content.php');
	if(($post = _content_select($args['id'])) == FALSE)
		return _error(INVALID_ARGUMENT);
	require_once('./system/user.php');
	if(!_user_admin($user_id) || $post['user_id'] != $user_id)
		return _error(PERMISSION_DENIED);
	print('<h1 class="title blog">Modification of blog post: '
		.$post['title']."</h1>\n");
	if(isset($args['preview']) && isset($args['title'])
			&& isset($args['content']))
	{
		$long = 1;
		$post = array('user_id' => $post['user_id'],
				'username' => $post['username']);
		$post['id'] = $args['id'];
		$post['tag'] = stripslashes($args['title']);
		$post['title'] = PREVIEW.': '.$post['tag'];
		$post['timestamp'] = stripslashes($args['timestamp']);
		$post['date'] = _sql_date($post['timestamp']);
		$post['content'] = stripslashes($args['content']);
		$post['preview'] = 1;
		include('./modules/blog/post_display.tpl');
		$post['title'] = stripslashes($args['title']);
	}
	include('./modules/blog/post_update.tpl');
}
}

?>
