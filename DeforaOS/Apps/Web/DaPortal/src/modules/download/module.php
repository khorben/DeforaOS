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
//- implement file deletion



require_once('./modules/content/module.php');


//DownloadModule
class DownloadModule extends ContentModule
{
	//essential
	//DownloadModule::DownloadModule
	public function __construct($id, $name)
	{
		parent::__construct($id, $name);
		$this->module_id = $id;
		$this->module_name = _('Downloads');
		//translations
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
		daportal_user.username AS username,
		daportal_content.content_id AS id, title,
		daportal_content.content AS content,
		timestamp, download.download_id AS download_id,
		parent.content_id AS parent_id, download.mode AS mode
		FROM daportal_content, daportal_module, daportal_user,
		daportal_download download
		LEFT JOIN daportal_download parent
		ON download.parent=parent.download_id
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=download.content_id
		AND daportal_content.enabled='1'
		AND (daportal_content.public='1' OR daportal_content.user_id=:user_id)
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND daportal_content.content_id=:content_id";
	protected $download_query_file_insert = 'INSERT INTO daportal_download
		(content_id, parent, mode) VALUES (:content_id, :parent,
			:mode)';
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
	protected function canSubmit($engine, $request = FALSE, $error)
	{
		$cred = $engine->getCredentials();

		if($cred->isAdmin())
			return TRUE;
		$error = _('Permission denied');
		return FALSE;
	}


	//DownloadModule::getRoot
	protected function getRoot()
	{
		global $config;

		if(($root = $config->getVariable('module::'.$this->name,
				'root')) === FALSE)
			$root = '/tmp';
		return $root;
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
				'stock' => 'upload', 'text' => _('Upload')));
		return $form;
	}


	//actions
	//DownloadModule::display
	protected function display($engine, $content)
	{
		return parent::display($engine, $content);
	}

	protected function _display($engine, $content)
	{
		$title = $this->content_item._(': ').$content['title'];

		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => $this->name,
				'text' => $title));
		//obtain the root repository
		$root = $this->getRoot();
		//output the file details
		$filename = $root.'/'.$content['download_id'];
		$stat = stat($filename);
		$this->_displayField($page, _('Permissions'),
			sprintf('%04o', $content['mode']));
		$this->_displayField($page, _('Creation time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['ctime']));
		$this->_displayField($page, _('Modification time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['mtime']));
		$this->_displayField($page, _('Access time'),
			strftime('%A, %B %e %Y, %H:%M:%S', $stat['atime']));
		$this->_displayField($page, _('Size'), $stat['size']);
		//link to the download
		$vbox = $page->append('vbox');
		$r = new Request($engine, $this->name, 'download',
				$content['id'], $content['title']);
		$vbox->append('button', array('request' => $r,
				'stock' => $this->name,
				'text' => _('Download')));
		//link to the folder
		//FIXME does not seem to work
		if(!is_numeric($content['parent_id']))
			$content['parent_id'] = FALSE;
		$r = new Request($engine, $this->name, FALSE,
				$content['parent_id']);
		$vbox->append('link', array('request' => $r,
				'stock' => 'back',
				'text' => _('Back')));
		return $page;
	}

	protected function _displayField($page, $label, $field)
	{
		$hbox = $page->append('hbox');
		$hbox->append('label', array('text' => $label._(': ')));
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
		$root = $this->getRoot();
		//output the file
		$filename = $root.'/'.$content['download_id'];
		//FIXME really implement (headers...)
		if(readfile($filename) !== FALSE)
			exit(0);
		$error = _('Could not read file');
		return new PageElement('dialog', array('type' => 'error',
				'text' => $error));
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
		if($engine->isIdempotent($request))
			return _('The request expired or is invalid');
		if($parent === FALSE)
			$parent = NULL;
		else if(!is_numeric($parent))
			return _('Invalid argument');
		//obtain the root repository
		$root = $this->getRoot();
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
