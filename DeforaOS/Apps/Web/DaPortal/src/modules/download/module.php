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



require_once('./system/content.php');
require_once('./modules/content/module.php');


//DownloadModule
class DownloadModule extends ContentModule
{
	//public
	//methods
	//forms
	//DownloadModule::form_file_insert
	protected function form_file_insert($engine, $request)
	{
		$parent = $request->getParameter('parent');

		if(!is_numeric($parent))
			$parent = FALSE;
		$r = new Request($engine, $this->name, 'file_insert');
		if($parent !== FALSE)
			//FIXME also tell where the upload is for
			$r->setParameter('parent', $parent);
		$form = new PageElement('form', array('request' => $r));
		$form->append('filechooser', array('name' => 'files[]'));
		$form->append('filechooser', array('name' => 'files[]'));
		$r = new Request($engine, $this->name, FALSE, $parent);
		$form->append('button', array('text' => _('Cancel'),
				'stock' => 'cancel', 'request' => $r));
		$form->append('button', array('type' => 'submit',
				'stock' => 'upload', 'text' => _('Upload')));
		return $form;
	}


	//essential
	//DownloadModule::DownloadModule
	public function __construct($id, $name)
	{
		parent::__construct($id, $name);
		$this->module_id = $id;
		$this->module_name = _('Downloads');
		//XXX check these
		$this->module_content = _('Download');
		$this->module_contents = _('Download');
		$this->content_open_text = _('Open');
		//list only files by default
		$this->query_list = $this->download_query_list_files;
	}


	//DownloadModule::call
	public function call(&$engine, $request)
	{
		switch(($action = $request->getAction()))
		{
			case 'download':
				return $this->download($engine, $request);
			case 'file_insert':
				return $this->fileInsert($engine, $request);
		}
		return parent::call($engine, $request);
	}


	//protected
	//properties
	//queries
	protected $download_query_list_files = "SELECT
		daportal_content.content_id AS id,
		timestamp, name AS module,
		daportal_user.user_id AS user_id, username, title
		FROM daportal_content, daportal_module, daportal_user,
		daportal_download
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.content_id=daportal_download.content_id
		AND daportal_content.enabled='1'
		AND daportal_content.public='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND mode & 512 = 0";


	//methods
	//DownloadModule::download
	protected function download($engine, $request)
	{
		//FIXME really implement
		return $this->preview($engine, $request);
	}


	//DownloadModule::fileInsert
	protected function fileInsert($engine, $request)
	{
		$title = _('File upload');
		$credentials = $engine->getCredentials();

		//check permissions
		if(!$credentials->isAdmin())
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('Permission denied')));
		$page = new Page(array('title' => $title));
		$page->append('title', array('stock' => 'upload',
			'text' => $title));
		//process the upload
		$error = $this->_fileInsert($engine, $request);
		//upload successful
		if($error === FALSE)
		{
			//FIXME really link to the parent directory
			$r = new Request($engine, $this->name);
			$page->setProperty('location', $engine->getUrl($r));
			$page->setProperty('refresh', 30);
			$box = $page->append('vbox');
			$text = _('Upload in progress, please wait...');
			$box->append('label', array('text' => $text));
			$box = $box->append('hbox');
			$text = _('If you are not redirected within 30 seconds, please ');
			$box->append('label', array('text' => $text));
			$box->append('link', array('text' => _('click here'),
					'request' => $r));
			$box->append('label', array('text' => '.'));
			return $page;
		}
		else if(is_string($error))
			$page->append('dialog', array('type' => 'error',
					'text' => $error));
		$form = $this->form_file_insert($engine, $request);
		$page->appendElement($form);
		return $page;
	}

	private function _fileInsert($engine, $request)
	{
		$db = $engine->getDatabase();

		//FIXME really lookup
		$root = '/tmp';
		if(!isset($_FILES['files']))
			return TRUE;
		if($engine->isIdempotent($request) !== FALSE)
			return _('The request expired or is invalid');
		$parent = $request->getParameter('parent');
		if($parent !== FALSE && !is_numeric($parent))
			return _('Invalid argument');
		var_dump($_FILES);
		foreach($_FILES['files']['error'] as $k => $v)
			if($v != UPLOAD_ERR_OK)
				return _('An error occurred');
		if($db->transactionBegin($engine) === FALSE)
			return _('Internal server error');
		foreach($_FILES['files']['error'] as $k => $v)
		{
			$name = $_FILES['files']['name'][$k];
			//FIXME check for filename unicity
			$content = Content::insert($engine, $this->id, $name,
					FALSE, TRUE, FALSE);
			if($content === FALSE)
			{
				$db->transactionRollback();
				return _('Internal server error');
			}
			$id = $content->getId();
			$tmp = $_FILES['files']['tmp_name'][$k];
			if(move_uploaded_file($tmp, $root.'/'.$id) !== TRUE)
			{
				$db->transactionRollback();
				return _('Internal server error');
			}
		}
		//FIXME really implement
		{
			$db->transactionRollback();
			return _('Not fully implemented');
		}
		if($db->transactionCommit($engine) === FALSE)
			return _('Internal server error');
		return FALSE;
	}
}

?>
