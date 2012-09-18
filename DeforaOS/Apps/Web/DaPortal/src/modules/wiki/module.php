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



require_once('./system/user.php');
require_once('./modules/content/module.php');


//WikiModule
class WikiModule extends ContentModule
{
	//public
	//methods
	//essential
	//WikiModule::WikiModule
	public function __construct($id, $name, $title = FALSE)
	{
		global $config;

		$this->root = $config->getVariable('module::wiki', 'root');
		$title = ($title === FALSE) ? _('Wiki') : $title;
		parent::__construct($id, $name);
		$this->text_content_admin = _('Wiki administration');
		$this->text_content_by = _('Page by');
		$this->text_content_item = _('Wiki page');
		$this->text_content_items = _('Wiki pages');
		$this->text_content_list_title = _('Wiki pages');
		$this->text_content_list_title_by = _('Wiki pages by');
		$this->text_content_more_content = _('More wiki pages...');
		$this->text_content_submit = _('New wiki page');
		$this->text_content_title = _('Wiki');
	}


	//protected
	//methods
	//accessors
	//WikiModule::canUpdate
	protected function canUpdate($engine, $content = FALSE, &$error = FALSE)
	{
		global $config;
		$cred = $engine->getCredentials();

		if($cred->getUserId() > 0)
			return TRUE;
		//anonymous users may be allowed to edit the wiki
		if($config->getVariable('module::'.$this->name, 'anonymous'))
			return TRUE;
		$error = _('Permission denied');
		return FALSE;
	}


	//WikiModule::_get
	protected function _get($engine, $id, $title = FALSE, $request = FALSE)
	{
		if(($content = parent::_get($engine, $id, $title)) === FALSE)
			return FALSE;
		if($this->root === FALSE
				|| strpos($content['title'], '/') !== FALSE)
			return $content; //XXX fail instead?
		$cmd = 'co -p -q';
		if($request !== FALSE && ($revision = $request->getParameter(
				'revision')) !== FALSE)
			$cmd .= ' -r'.escapeshellarg($revision);
		$cmd .= ' '.escapeshellarg($this->root.'/'.$content['title']);
		exec($cmd, $rcs, $res);
		if($res != 0)
			return FALSE;
		$rcs = implode("\n", $rcs);
		$content['content'] = $rcs;
		return $content;
	}


	//calls
	//WikiModule::callDefault
	protected function callDefault($engine, $request = FALSE)
	{
		$title = $this->text_content_title;

		if($request !== FALSE && $request->getId() !== FALSE)
			return $this->callDisplay($engine, $request);
		$page = new Page(array('title' => $title));
		$page->append('title', array('text' => $title,
				'stock' => $this->name));
		$vbox = $page->append('vbox');
		//search
		$vbox->append('title', array('text' => _('Search the wiki'),
				'stock' => 'search'));
		$r = new Request('search', 'advanced', FALSE, FALSE,
			array('inmodule' => $this->name, 'intitle' => 1));
		$form = $vbox->append('form', array('request' => $r));
		$hbox = $form->append('hbox');
		$hbox->append('entry', array('name' => 'q',
				'text' => _('Look for a page: ')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'search', 'text' => _('Search')));
		$r = new Request('search', 'advanced', FALSE, FALSE,
			array('inmodule' => $this->name, 'incontent' => 1));
		$form = $vbox->append('form', array('request' => $r));
		$hbox = $form->append('hbox');
		$hbox->append('entry', array('name' => 'q',
				'text' => _('Look inside pages: ')));
		$hbox->append('button', array('type' => 'submit',
				'stock' => 'search', 'text' => _('Search')));
		$vbox->append('title', array('text' => _('Recent changes'),
				'stock' => 'help'));
		//recent changes
		$vbox->append($this->callHeadline($engine, FALSE));
		//page list
		$r = new Request($this->name, 'list');
		$vbox->append('link', array('request' => $r,
				'stock' => $this->name,
				'text' => _('List all pages')));
		return $page;
	}


	//helpers
	//WikiModule::helperDisplay
	protected function helperDisplay($engine, $page, $content = FALSE)
	{
		if(($page = parent::helperDisplay($engine, $page, $content))
				=== FALSE)
			return FALSE;
		//link
		$request = new Request($content['module'], FALSE,
				$content['id'], $content['title']);
		$this->helperDisplayRevisions($engine, $page, $request,
				$content);
		return $page;
	}


	//WikiModule::helperDisplayRevisions
	protected function helperDisplayRevisions($engine, $page, $request,
			$content)
	{
		$error = _('Could not list revisions');

		if($this->root === FALSE
				|| strpos($content['title'], '/') !== FALSE)
			return $page->append('dialog', array('type' => 'error',
					'text' => $error));
		//obtain the revision list
		$cmd = 'rlog';
		$cmd .= ' '.escapeshellarg($this->root.'/'.$content['title']);
		exec($cmd, $rcs, $res);
		if($res != 0)
			return $page->append('dialog', array('type' => 'error',
					'text' => $error));
		for($i = 0, $cnt = count($rcs); $i < $cnt;)
			if($rcs[$i++] == '----------------------------')
				break;
		$columns = array('title' => _('Name'), 'date' => _('Date'),
				'username' => _('Author'),
				'message' => _('Message'));
		$vbox = $page->append('vbox');
		$vbox->append('title', array('text' => 'Revisions',
				'stock' => $this->name));
		$view = $vbox->append('treeview', array('columns' => $columns));
		$lsp = '======================================================';
		$ssp = '----------------------------';
		for(; $i < $cnt - 2; $i += 3)
		{
			$row = $view->append('row');
			//name
			$name = substr($rcs[$i], 9);
			$r = new Request($this->name, FALSE, $content['id'],
			       	$content['title'], array('revision' => $name));
			$name = new PageElement('link', array('request' => $r,
					'text' => $name));
			$row->setProperty('title', $name);
			//date
			$date = substr($rcs[$i + 1], 6, 19);
			$row->setProperty('date', $date);
			//username
			$username = substr($rcs[$i + 1], 36);
			$username = substr($username, 0, strspn($username,
					'abcdefghijklmnopqrstuvwxyz'
					.'ABCDEFGHIJKLMNOPQRSTUV'
					.'WXYZ0123456789'));
			if(($user = User::lookup($engine, $username)) !== FALSE)
			{
				$r = new Request('user', FALSE,
					$user->getUserId(),
					$user->getUsername());
				$username = new PageElement('link', array(
						'request' => $r,
						'stock' => 'user',
						'text' => $username));
			}
			$row->setProperty('username', $username);
			//message
			$message = $rcs[$i + 2];
			if($message == $ssp || strncmp($message, $lsp,
					strlen($lsp)) == 0)
				$message = '';
			else
			{
				$apnd = '';
				for($i++; $i < $cnt && $rcs[$i + 2] != $ssp
					&& strncmp($rcs[$i + 2], $lsp,
						strlen($lsp)) != 0; $i++)
						$apnd = '...';
				$message .= $apnd;
			}
			$row->setProperty('message', $message);
		}
	}


	//WikiModule::helperDisplayText
	protected function helperDisplayText($engine, $page, $request, $content)
	{
		$page->append('htmlview', array('text' => $content['content']));
	}


	//WikiModule::helperPreviewText
	protected function helperPreviewText($engine, $page, $request, $content)
	{
		$page->append('htmlview', array('text' => $content['content']));
	}


	//WikiModule::helperSubmitContent
	protected function helperSubmitContent($engine, $request, $page)
	{
		$value = $request->getParameter('content');

		$page->append('htmledit', array('name' => 'content',
				'value' => $value));
	}


	//WikiModule::helperUpdateContent
	protected function helperUpdateContent($engine, $request, $page,
			$content)
	{
		$page->append('label', array('text' => _('Content: ')));
		if(($value = $request->getParameter('content')) === FALSE)
			$value = $content['content'];
		$page->append('htmledit', array('name' => 'content',
				'value' => $value));
		$message = $request->getParameter('message');
		$page->append('entry', array('name' => 'message',
				'text' => _('Log message: '),
				'value' => $message));
	}


	//private
	//properties
	private $root = FALSE;
}

?>
