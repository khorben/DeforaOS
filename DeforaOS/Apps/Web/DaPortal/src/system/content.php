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



//Content
class Content
{
	//public
	//methods
	//accessors
	//Content::getContent
	public function getContent()
	{
		return $this->content;
	}


	//Content::getId
	public function getId()
	{
		return $this->id;
	}


	//Content::getTitle
	public function getTitle()
	{
		return $this->title;
	}


	//static
	//useful
	//Content::delete
	static public function delete($engine, $module_id, $content_id)
	{
		$db = $engine->getDatabase();
		$query = Content::$query_delete;

		if(!is_numeric($module_id) || !is_numeric($content_id))
			return $engine->log('LOG_ERR',
					'Invalid content to delete');
		if($db->query($engine, $query, array('module_id' => $module_id,
				'content_id' => $content_id)) === FALSE)
			return $engine->log('LOG_ERR',
					'Could not delete content');
		return TRUE;
	}


	//Content::get
	static public function get($engine, $module_id, $id, $title = FALSE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();

		$query = Content::$query_get;
		$args = array('module_id' => $module_id, 'content_id' => $id,
				'user_id' => $cred->getUserId());
		if(is_string($title))
		{
			$query .= ' AND title LIKE :title';
			$args['title'] = str_replace('-', '_', $title);
		}
		if(($res = $db->query($engine, $query, $args)) === FALSE
				|| count($res) != 1)
			return $engine->log('LOG_ERR',
					'Could not fetch content');
		$res = $res[0];
		return new Content($res['id'], $res['module_id'],
			$res['title'], $res['content'], $res['enabled'],
			$res['public']);
	}


	//Content::insert
	static public function insert($engine, $module_id, $title = FALSE,
			$content = FALSE, $public = FALSE, $enabled = TRUE)
	{
		$cred = $engine->getCredentials();
		$db = $engine->getDatabase();
		$query = Content::$query_insert;

		if(!is_numeric($module_id))
			//XXX make a stricter check
			return $engine->log('LOG_ERR',
					'Invalid module for content to insert');
		if($content === FALSE)
			$content = '';
		if(!is_string($title) || !is_string($content)
				|| !is_bool($enabled) || !is_bool($public))
			return $engine->log('LOG_ERR',
					'Invalid content to insert');
		if($db->query($engine, $query, array('module_id' => $module_id,
				'user_id' => $cred->getUserId(),
				'title' => $title, 'content' => $content,
				'enabled' => $enabled, 'public' => $public))
				=== FALSE)
			return $engine->log('LOG_ERR',
					'Could not insert content');
		$id = $db->getLastId($engine, 'daportal_content', 'content_id');
		$content = new Content($id, $module_id, $title, $content,
			$enabled, $public);
		return $content;
	}


	//Content::update
	static public function update($engine, $content_id, $title,
			$content = FALSE)
	{
		$db = $engine->getDatabase();
		$query = Content::$query_update_title;

		if(!is_numeric($content_id) || !is_string($title)
				|| ($content !== FALSE && !is_string($content)))
			return $engine->log('LOG_ERR',
					'Invalid content to update');
		$args = array('content_id' => $content_id,
				'title' => $title);
		if($content !== FALSE)
		{
			$query = Content::$query_update;
			$args['content'] = $content;
		}
		if($db->query($engine, $query, $args) === FALSE)
			return $engine->log('LOG_ERR',
					'Could not update content');
		return TRUE;
	}


	//private
	//properties
	private $id = FALSE;
	private $module_id = FALSE;
	private $title = FALSE;
	private $content = FALSE;
	private $enabled = FALSE;
	private $public = FALSE;

	//queries
	static private $query_delete = 'DELETE FROM daportal_content
		WHERE module_id=:module_id AND content_id=:content_id';
	static private $query_get = "SELECT daportal_module.name AS module,
		daportal_user.user_id AS user_id,
		daportal_user.username AS username,
		daportal_content.content_id AS id, title, content, timestamp
		FROM daportal_content, daportal_module, daportal_user
		WHERE daportal_content.module_id=daportal_module.module_id
		AND daportal_content.module_id=:module_id
		AND daportal_content.user_id=daportal_user.user_id
		AND daportal_content.enabled='1'
		AND (daportal_content.public='1' OR daportal_content.user_id=:user_id)
		AND daportal_module.enabled='1'
		AND daportal_user.enabled='1'
		AND content_id=:content_id";
	static private $query_insert = 'INSERT INTO daportal_content
		(module_id, user_id, title, content, enabled, public)
		VALUES (:module_id, :user_id, :title, :content, :enabled,
			:public)';
	static private $query_update = 'UPDATE daportal_content
		SET title=:title, content=:content
		WHERE content_id=:content_id';
	static private $query_update_title = 'UPDATE daportal_content
		SET title=:title
		WHERE content_id=:content_id';


	//methods
	//essential
	//Content::Content
	private function __construct($id, $module_id, $title, $content,
		$enabled, $public)
	{
		$this->id = $id;
		$this->module_id = $module_id;
		$this->title = $title;
		$this->content = $content;
		$this->enabled = $enabled;
		$this->public = $public;
	}

}

?>
