<?php //$Id$
//Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DeforaOS Web DaPortal
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//FIXME:
//- properly implement file deletion



require_once('./system/mime.php');
require_once('./modules/content/module.php');


//DownloadModule
class DownloadModule extends ContentModule
{
	//essential
	//DownloadModule::DownloadModule
	public function __construct($id, $name, $title = FALSE)
	{
		$title = ($title === FALSE) ? _('Downloads') : $title;
		parent::__construct($id, $name, $title);
		//translations
		$this->content_admin = _('Downloads administration');
		$this->content_by = _('Download by');
		$this->content_item = _('Download');
		$this->content_items = _('Downloads');
		$this->content_list_title = _('Directory listing');
		$this->content_open_text = _('Open');
		$this->content_more_content = _('Back to directory listing...');
		$this->content_submit = _('File upload');
		$this->content_submit_progress
			= _('Upload in progress, please wait...');
		$this->content_title = _('Latest downloads');
		//queries
		$this->query_get = $this->download_query_get;
		//list only files by default
		$this->query_list = $this->download_query_list_files;
		$this->query_list_count
			= $this->download_query_list_files_count;
	}


	//DownloadModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'download':
				return $this->download($engine, $request);
			case 'folder_new':
				return $this->folder_new($engine, $request);
			case 'file_insert':
			case 'submit':
				return $this->submit($engine, $request);
		}
		return parent::call($engine, $request);
	}


	//protected
	//properties
	//queries
	protected $download_query_get = "SELECT daportal_module.name AS module,
		daportal_user.user_id AS user_id,
		daportal_user.username AS username,
		daportal_group.group_id AS group_id,
		daportal_group.groupname AS groupname,
		daportal_content.content_id AS id,
		daportal_content.title AS title,
		daportal_content.content AS content,
		daportal_content.timestamp AS timestamp,
		download.download_id AS download_id,
		parent_download.content_id AS parent_id,
		parent_content.title AS parent_title,
		download.mode AS mode
		FROM daportal_content, daportal_module, daportal_user,
		daportal_group, daportal_download download
		LEFT JOIN daportal_download parent_download
		ON download.parent=parent_download.download_id
		LEFT JOIN daportal_content parent_content
		ON parent_download.content_id=parent_content.content_id
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.group_id=daportal_group.group_id
		AND daportal_content.content_id=download.content_id
		AND daportal_content.enabled='1'
		AND (daportal_content.public='1' OR daportal_content.user_id=:user_id)
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_content.content_id=:content_id";
	protected $download_query_get_download_id = "SELECT download_id
		FROM daportal_download, daportal_content
		WHERE daportal_download.content_id=daportal_content.content_id
		AND daportal_content.enabled='1'
		AND daportal_download.content_id=:content_id";
	protected $download_query_file_insert = 'INSERT INTO daportal_download
		(content_id, parent, mode) VALUES (:content_id, :parent,
			:mode)';
	protected $download_query_list = "SELECT
		daportal_content.content_id AS id,
		daportal_content.enabled AS enabled,
		daportal_content.timestamp AS timestamp,
		daportal_user.user_id AS user_id, username,
		daportal_group.group_id AS group_id, groupname, title, mode
		FROM daportal_content, daportal_module, daportal_user,
		daportal_group, daportal_download
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.group_id=daportal_group.group_id
		AND daportal_content.content_id=daportal_download.content_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	protected $download_query_list_files = "SELECT
		daportal_content.content_id AS id,
		daportal_content.enabled AS enabled,
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title
		FROM daportal_content, daportal_module, daportal_user,
		daportal_download
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_download.content_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND mode & 512 = 0";
	protected $download_query_list_files_count = "SELECT COUNT(*)
		FROM daportal_content, daportal_module, daportal_user,
		daportal_download
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_download.content_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND mode & 512 = 0";


	//methods
	//accessors
	//DownloadModule::canSubmit
	protected function canSubmit($engine, $request = FALSE, &$error = FALSE)
	{
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			return TRUE;
		$error = _('Permission denied');
		return FALSE;
	}


	//DownloadModule::getDownloadId
	protected function getDownloadId($engine, $content_id)
	{
		$db = $engine->getDatabase();

		$query = $this->download_query_get_download_id;
		if(($res = $db->query($engine, $query, array(
					'content_id' => $content_id))) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0]['download_id'];
	}


	//DownloadModule::getPermissionsString
	protected function getPermissionsString($mode)
	{
		$str = '----------';
		if(($mode & 512) == 512)
			$str[0] = 'd';
		$str[1] = $mode & 0400 ? 'r' : '-';
		$str[2] = $mode & 0200 ? 'w' : '-';
		$str[3] = $mode & 0100 ? 'x' : '-';
		$str[4] = $mode & 040 ? 'r' : '-';
		$str[5] = $mode & 020 ? 'w' : '-';
		$str[6] = $mode & 010 ? 'x' : '-';
		$str[7] = $mode & 04 ? 'r' : '-';
		$str[8] = $mode & 02 ? 'w' : '-';
		$str[9] = $mode & 01 ? 'x' : '-';
		return $str;
	}


	//DownloadModule::getRoot
	protected function getRoot($engine)
	{
		global $config;

		if(($root = $config->getVariable('module::'.$this->name,
				'root')) === FALSE)
		{
			$engine->log('LOG_WARNING',
				'The download repository is not configured');
			$root = '/tmp';
		}
		return $root;
	}


	//DownloadModule::isDirectory
	protected function isDirectory($content)
	{
		return ($content['mode'] & 01000) ? TRUE : FALSE;
	}


	//forms
	//DownloadModule::formSubmit
	protected function formSubmit($engine, $request)
	{
		$parent = $request->getParameter('parent');

		if(!is_numeric($parent))
			$parent = FALSE;
		$r = new Request($engine, $this->name, 'submit');
		if($parent !== FALSE)
			//FIXME also tell where the upload is for
			$r->setParameter('parent', $parent);
		$form = new PageElement('form', array('request' => $r));
		$form->append('filechooser', array('text' => _('File: '),
				'name' => 'files[]'));
		$r = new Request($engine, $this->name, FALSE, $parent);
		$form->append('button', array('text' => _('Cancel'),
				'stock' => 'cancel', 'request' => $r));
		$form->append('button', array('type' => 'submit',
				'stock' => 'upload', 'name' => 'submit',
				'text' => _('Upload')));
		return $form;
	}


	//actions
	//DownloadModule::_default
	protected function _default($engine, $request = FALSE)
	{
		if($request === FALSE || ($id = $request->getId()) === FALSE)
		{
			$content = array('id' => FALSE,
				'title' => 'Root directory');
			return $this->_displayDirectory($engine, $content);
		}
		return $this->display($engine, $request);
	}


	//DownloadModule::display
	protected function display($engine, $request)
	{
		return parent::display($engine, $request);
	}

	protected function _display($engine, $content)
	{
		if($this->isDirectory($content))
			return $this->_displayDirectory($engine, $content);
		return $this->_displayFile($engine, $content);
	}

	protected function _displayDirectory($engine, $content)
	{
		$title = $this->content_list_title._(': ').$content['title'];
		$db = $engine->getDatabase();
		$query = $this->download_query_list;

		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		$parent_id = (is_numeric($content['id']))
			? $this->getDownloadId($engine, $content['id'])
			: FALSE;
		if($parent_id !== FALSE)
			$query .= ' AND daportal_download.parent=:parent_id';
		else
			$query .= ' AND daportal_download.parent IS NULL';
		$query .= ' ORDER BY title ASC';
		if(($res = $db->query($engine, $query, array(
					'module_id' => $this->id,
					'parent_id' => $parent_id))) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => _('Unable to list files')));
		//toolbar
		$toolbar = $page->append('toolbar');
		//link to the parent folder
		$parent_id = isset($content['parent_id'])
			&& is_numeric($content['parent_id'])
			? $content['parent_id'] : FALSE;
		$parent_title = isset($content['parent_title'])
			&& is_string($content['parent_title'])
			? $content['parent_title'] : FALSE;
		$request = new Request($engine, $this->name, FALSE, $parent_id,
				$parent_title);
		$toolbar->append('button', array('request' => $request,
				'stock' => 'updir',
				'text' => _('Up one directory')));
		$request = new Request($engine, $this->name, 'folder_new',
				$parent_id, $parent_title);
		$toolbar->append('button', array('request' => $request,
				'stock' => 'folder-new',
				'text' => _('New directory')));
		//view
		$columns = array('filename' => _('Filename'),
			'owner' => _('Owner'), 'group' => _('Group'),
			'date' => _('Date'), 'permissions' => _('Permissions'));
		$view = $page->append('treeview', array('columns' => $columns));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$stock = $this->isDirectory($res[$i])
					? 'folder' : 'file';
			$row = $view->append('row');
			$row->setProperty('id', $res[$i]['id']);
			$r = new Request($engine, $this->name, FALSE,
					$res[$i]['id'], $res[$i]['title']);
			$link = new PageElement('link', array('stock' => $stock,
					'request' => $r,
					'text' => $res[$i]['title']));
			$row->setProperty('filename', $link);
			$user_id = $res[$i]['user_id'];
			$username = $res[$i]['username'];
			if(is_numeric($user_id) && $user_id != 0)
			{
				$r = new Request($engine, 'user', FALSE,
					$user_id, $username);
				$username = new PageElement('link', array(
					'request' => $r, 'text' => $username));
			}
			$row->setProperty('owner', $username);
			$row->setProperty('group', $res[$i]['groupname']);
			$row->setProperty('date', $this->_timestampToDate(
					$res[$i]['timestamp'],
						_('d/m/Y H:i:s')));
			$row->setProperty('permissions',
				$this->getPermissionsString($res[$i]['mode']));
		}
		return $page;
	}

	protected function _displayFile($engine, $content)
	{
		$title = $this->content_item._(': ').$content['title'];

		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//toolbar
		$toolbar = $page->append('toolbar');
		//link to the folder
		$parent_id = is_numeric($content['parent_id'])
			? $content['parent_id'] : FALSE;
		$parent_title = is_string($content['parent_title'])
			? $content['parent_title'] : FALSE;
		$request = new Request($engine, $this->name, FALSE, $parent_id,
				$parent_title);
		$toolbar->append('button', array('request' => $request,
				'stock' => 'updir', 'text' => _('Browse')));
		//link to the download
		$request = new Request($engine, $this->name, 'download',
				$content['id'], $content['title']);
		$toolbar->append('button', array('request' => $request,
				'stock' => $this->name,
				'text' => _('Download')));
		//obtain the root repository
		$root = $this->getRoot($engine);
		//output the file details
		$filename = $root.'/'.$content['download_id'];
		$error = _('Could not obtain details for this file');
		if(($stat = stat($filename)) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		$this->_displayField($page, _('Name'), new PageElement('link',
					array('request' => $request,
						'text' => $content['title'])));
		$this->_displayField($page, _('Type'), Mime::get($engine,
				$content['title']));
		$request = new Request($engine, 'user', FALSE,
				$content['user_id'], $content['username']);
		$this->_displayField($page, _('Owner'), new PageElement('link',
					array('request' => $request,
					'text' => $content['username'])));
		$this->_displayField($page, _('Group'), $content['groupname']);
		$this->_displayField($page, _('Permissions'),
			$this->getPermissionsString($content['mode']));
		$this->_displayField($page, _('Creation time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['ctime']));
		$this->_displayField($page, _('Modification time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['mtime']));
		$this->_displayField($page, _('Access time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['atime']));
		$this->_displayField($page, _('Size'), $stat['size']);
		$this->_displayField($page, _('Comment'), $content['content']);
		return $page;
	}

	protected function _displayField($page, $label, $field)
	{
		$hbox = $page->append('hbox');
		$hbox->append('label', array('text' => $label._(': ')));
		if($field instanceof PageElement)
			$hbox->append($field);
		else
			$hbox->append('label', array('text' => $field));
	}


	//DownloadModule::download
	protected function download($engine, $request)
	{
		global $config;
		$error = _('Could not fetch content');

		if(($id = $request->getId()) === FALSE)
			return $this->_default($engine);
		if(($content = $this->_get($engine, $id, $request->getTitle()))
				=== FALSE)
			return new PageElement('dialog', array(
					'type' => 'error',
					'text' => $error));
		//obtain the root repository
		$root = $this->getRoot($engine);
		//output the file
		$filename = $root.'/'.$content['download_id'];
		$mime = Mime::get($engine, $content['title']);
		if(($fp = fopen($filename, 'rb')) === FALSE)
		{
			$error = _('Could not read file');
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => $error));
		}
		$engine->setType($mime);
		return $fp;
	}


	//DownloadModule::folder_new
	protected function folder_new($engine, $request)
	{
		$title = _('New folder');

		//FIXME process the input if any
		if($request->getId() === FALSE)
			$title = _('New root folder');
		else if(($t = $request->getTitle()) !== FALSE)
			$title = sprintf(_('New folder in "%s"'), $t);
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'folder-new',
				'text' => $title));
		$r = new Request($engine, $this->name, 'folder_new',
				$request->getId(), $request->getTitle());
		$form = $page->append('form', array('request' => $r));
		$name = $form->append('entry', array('text' => _('Name: '),
				'name' => 'name',
				'value' => $request->getParameter('name')));
		$r = new Request($engine, $this->name, FALSE,
				$request->getId(), $request->getTitle());
		$form->append('button', array('stock' => 'cancel',
				'request' => $r, 'text' => _('Cancel')));
		$form->append('button', array('stock' => 'folder-new',
				'type' => 'submit', 'text' => _('Create')));
		return $page;
	}


	//DownloadModule::submit
	protected function submit($engine, $request)
	{
		return parent::submit($engine, $request);
	}

	protected function _submitProcess($engine, $request, $parent)
	{
		global $config;
		$db = $engine->getDatabase();

		//verify the request
		if(!isset($_FILES['files']))
			return TRUE;
		if($request->isIdempotent() !== FALSE)
			return _('The request expired or is invalid');
		if($parent === FALSE)
			$parent = NULL;
		else if(!is_numeric($parent))
			return _('Invalid argument');
		//obtain the root repository
		$root = $this->getRoot($engine);
		//check known errors
		foreach($_FILES['files']['error'] as $k => $v)
			if($v != UPLOAD_ERR_OK)
				return _('An error occurred');
		//store each file uploaded
		foreach($_FILES['files']['error'] as $k => $v)
		{
			if($db->transactionBegin($engine) === FALSE)
				return _('Internal server error');
			$name = $_FILES['files']['name'][$k];
			//FIXME check for filename unicity
			$content = Content::insert($engine, $this->id, $name,
					FALSE, TRUE, TRUE);
			if($content === FALSE)
			{
				$db->transactionRollback();
				return _('Internal server error');
			}
			$id = $content->getId();
			$tmp = $_FILES['files']['tmp_name'][$k];
			$query = $this->download_query_file_insert;
			if($db->query($engine, $query, array(
						'content_id' => $id,
						'parent' => $parent,
						'mode' => 420)) === FALSE)
			{
				$db->transactionRollback();
				return _('Internal server error');
			}
			//store the file
			$id = $db->getLastId($engine, 'daportal_download',
					'download_id');
			$dst = $root.'/'.$id;
			if(move_uploaded_file($tmp, $dst) !== TRUE)
			{
				$db->transactionRollback();
				return _('Internal server error');
			}
			if($db->transactionCommit($engine) === FALSE)
			{
				if(file_exists($dst))
					unlink($dst);
				return _('Internal server error');
			}
		}
		return FALSE;
	}

	protected function _submitSuccess($engine, $request, $page, $parent)
	{
		$r = new Request($engine, $this->name, FALSE, $parent);
		$page->setProperty('location', $engine->getUrl($r));
		$page->setProperty('refresh', 30);
		$box = $page->append('vbox');
		$text = $this->content_submit_progress;
		$box->append('label', array('text' => $text));
		$box = $box->append('hbox');
		$text = _('If you are not redirected within 30 seconds, please ');
		$box->append('label', array('text' => $text));
		$box->append('link', array('text' => _('click here'),
			'request' => $r));
		$box->append('label', array('text' => '.'));
		return $page;
	}
}

?>
