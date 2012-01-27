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



require_once('./system/module.php');


//ContentModule
class ContentModule extends Module
{
	//public
	//methods
	//ContentModule::call
	public function call(&$engine, $request)
	{
		switch($request->getAction())
		{
			case 'admin':
				return $this->admin($engine);
			case 'list':
				return $this->_list($engine, $request);
			case 'preview':
				return $this->preview($engine,
						$request->getId(),
						$request->getTitle());
			default:
				return $this->_default($engine, $request);
		}
	}


	//protected
	//properties
	protected $module_id = FALSE;
	protected $module_name = 'Content';


	//ContentModule::admin
	protected function admin($engine)
	{
		$cred = $engine->getCredentials();

		$page = new Page;
		if(!$cred->isAdmin())
		{
			$engine->log('LOG_ERR', 'Permission denied');
			$r = new Request($engine, 'user', 'login');
			$dialog = $page->append('dialog', array(
						'type' => 'error',
						'text' => 'Permission denied'));
			$dialog->append('button', array('stock' => 'login',
						'text' => 'Login',
						'request' => $r));
			return $page;
		}
		$title = $this->module_name.' administration';
		$page->setProperty('title', $title);
		$element = $page->append('title');
		$db = $engine->getDatabase();
		$query = $this->query_list_admin;
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		if(($res = $db->query($engine, $query)) === FALSE)
			//FIXME return a dialog instead
			return $engine->log('LOG_ERR',
					'Unable to list contents');
		$element->setProperty('text', $title);
		$treeview = $page->append('treeview');
		$treeview->setProperty('columns', array('title', 'enabled',
					'username', 'date'));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
		}
		return $page;
	}


	//ContentModule::_default
	protected function _default($engine, $request)
	{
		if(($id = $request->getId()) !== FALSE)
			return $this->_display($engine, $id,
					$request->getTitle());
		return $this->_list($engine, $request);
	}


	//ContentModule::list
	protected function _list($engine, $request)
	{
		$page = new Page;
		$page->append('title', array('text' => $this->module_name));
		//FIXME really implement
		$db = $engine->getDatabase();
		$query = $this->query_list;
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		if(($res = $db->query($engine, $query)) === FALSE)
			//FIXME return a dialog instead
			return $engine->log('LOG_ERR',
					'Unable to list contents');
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
			$page->appendElement($this->_preview($engine,
						$res[$i]['id']));
		return $page;
	}


	//ContentModule::preview
	protected function preview($engine, $id, $title = FALSE)
	{
		$page = new Page;
		$page->appendElement($this->_preview($engine, $id, $title));
		return $page;
	}


	//private
	//properties
	private $query_get = "SELECT daportal_module.name AS module,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content AS text
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND content_id=:id";
	private $query_list = "SELECT content_id AS id
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";
	private $query_list_admin = "SELECT content_id AS id, timestamp AS date,
		name AS module, daportal_user.user_id AS user_id, username,
		title, daportal_content.enabled AS enabled
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'";


	//methods
	//ContentModule::_display
	private function _display($engine, $id, $title = FALSE)
	{
		if($id === FALSE)
			return $this->default();
		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return FALSE;
		//FIXME display metadata and link to actual resource?
		$page = new Page;
		$page->setProperty('title', $content['title']);
		$element = $page->append('label');
		$element->setProperty('text', $content['text']);
		return $page;
	}


	//ContentModule::_get
	private function _get($engine, $id, $title = FALSE)
	{
		if($id === FALSE)
			return FALSE;
		$db = $engine->getDatabase();
		$query = $this->query_get;
		$args = array('id' => $id);
		if($title !== FALSE)
		{
			$query .= ' AND title LIKE :title';
			$args['title'] = str_replace('-', '_', $title);
		}
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return FALSE;
		return $res[0];
	}

	
	//ContentModule::_preview
	private function _preview($engine, $id, $title = FALSE)
	{
		if(($content = $this->_get($engine, $id, $title)) === FALSE)
			return new PageElement('dialog', array(
						'title' => 'error',
						'text' => 'Could not fetch content'));
		$page = new PageElement('vbox');
		$r = new Request($engine, $content['module'], FALSE,
				$content['id']);
		$page->setProperty('text', $content['title']);
		$page->append('title', array('text' => $content['title']));
		$hbox = $page->append('hbox');
		$hbox->append('image', array('stock' => 'module '
					.$content['module'].' content'));
		$hbox->append('label', array('text' => $content['text']));
		$page->append('button', array('stock' => 'read',
					'text' => 'Read', 'request' => $r));
		return $page;
	}
}

?>
