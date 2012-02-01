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
		switch(($action = $request->getAction()))
		{
			case 'admin':
			case 'delete':
			case 'disable':
			case 'enable':
				return $this->$action($engine, $request);
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


	//methods
	//ContentModule::_apply
	protected function _apply($engine, $request, $query, $fallback,
			$success, $failure)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		if(($uid = $cred->getUserId()) == 0)
		{
			//must be logged in
			$page = $this->_default($engine);
			$page->prepend('dialog', array('type' => 'error',
						'text' => 'Must be logged in'));
			return $page;
		}
		if($engine->isIdempotent($request))
			//must be safe
			return $this->$fallback($engine);
		$type = 'info';
		$message = $success;
		$parameters = $request->getParameters();
		print_r($parameters);
		foreach($parameters as $k => $v)
		{
			$x = explode(':', $k);
			if(count($x) != 2 || $x[0] != 'content_id'
					|| !is_numeric($x[1]))
				continue;
			$res = $db->query($engine, $query, array(
						'content_id' => $x[1],
						'user_id' => $uid));
			if($res !== FALSE)
				continue;
			$type = 'error';
			$message = $failure;
		}
		$page = $this->$fallback($engine);
		$page->prepend('dialog', array('type' => $type,
					'text' => $message));
		return $page;
	}


	//ContentModule::admin
	protected function admin($engine, $request = FALSE)
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
		$r = new Request($engine, 'content', 'admin'); //XXX
		$treeview = $page->append('treeview', array('request' => $r));
		$treeview->setProperty('columns', array('title', 'enabled',
					'username', 'date'));
		$toolbar = $treeview->append('toolbar');
		$toolbar->append('button', array('stock' => 'disable',
					'text' => 'Disable',
					'type' => 'submit', 'name' => 'action',
					'value' => 'disable'));
		$toolbar->append('button', array('stock' => 'enable',
					'text' => 'Enable',
					'type' => 'submit', 'name' => 'action',
					'value' => 'enable'));
		for($i = 0, $cnt = count($res); $i < $cnt; $i++)
		{
			$row = $treeview->append('row');
			$row->setProperty('id', 'content_id:'.$res[$i]['id']);
			$row->setProperty('title', $res[$i]['title']);
			$row->setProperty('enabled', $res[$i]['enabled']);
			$row->setProperty('username', $res[$i]['username']);
			$row->setProperty('date', $res[$i]['date']);
		}
		return $page;
	}


	//ContentModule::_default
	protected function _default($engine, $request = FALSE)
	{
		if($request !== FALSE && ($id = $request->getId()) !== FALSE)
			return $this->_display($engine, $id,
					$request->getTitle());
		//FIXME run _list() here and let it manage one's content instead
		return $this->_list($engine, $request);
	}


	//ContentModule::disable
	protected function disable($engine, $request)
	{
		return $this->_apply($engine, $request, $this->query_disable,
				'admin',
				'Content could be disabled successfully',
				'Some content could not be disabled ');
	}


	//ContentModule::enable
	protected function enable($engine, $request)
	{
		return $this->_apply($engine, $request, $this->query_enable,
				'admin',
				'Content could be enabled successfully',
				'Some content could not be enabled ');
	}


	//ContentModule::list
	protected function _list($engine, $request = FALSE)
	{
		$page = new Page;
		$page->append('title', array('text' => $this->module_name));
		//FIXME really implement
		$db = $engine->getDatabase();
		$query = $this->query_list;
		if($this->module_id !== FALSE)
			$query .= " AND daportal_module.module_id='"
				.$this->module_id."'";
		$query .= ' ORDER BY timestamp DESC';
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
	private $query_disable = "UPDATE daportal_content
		SET enabled='0'
		WHERE content_id=:content_id AND user_id=:user_id";
	private $query_enable = "UPDATE daportal_content
		SET enabled='1'
		WHERE content_id=:content_id AND user_id=:user_id";
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
		$page->append('title', array('text' => $content['title']));
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
