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
	//accessors
	//getId
	public function getId()
	{
		return $this->id;
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
			return FALSE;
		if($content === FALSE)
			$content = '';
		if(!is_string($title) || !is_string($content)
				|| !is_bool($enabled) || !is_bool($public))
			return FALSE;
		if($db->query($engine, $query, array('module_id' => $module_id,
				'user_id' => $cred->getUserId(),
				'title' => $title, 'content' => $content,
				'enabled' => $enabled, 'public' => $public))
				=== FALSE)
			return FALSE;
		$id = $db->getLastId($engine, 'daportal_content', 'content_id');
		$content = new Content();
		$content->id = $id;
		return $content;
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
	static private $query_insert = 'INSERT INTO daportal_content
		(module_id, user_id, title, content, enabled, public)
		VALUES (:module_id, :user_id, :title, :content, :enabled,
			:public)';

}

?>
