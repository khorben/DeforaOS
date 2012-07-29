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



require_once('./system/format.php');


//AtomFormat
class AtomFormat extends Format
{
	//methods
	//essential
	//AtomFormat::match
	protected function match(&$engine, $type = FALSE)
	{
		switch($type)
		{
			case 'application/rss+xml':
				return 100;
			default:
				return 0;
		}
	}


	//AtomFormat::attach
	protected function attach(&$engine, $type = FALSE)
	{
		//for escaping
		if(!defined('ENT_XML1'))
			define('ENT_XML1', 0);
	}


	//public
	//methods
	//rendering
	//AtomFormat::render
	public function render(&$engine, $page, $filename = FALSE)
	{
		global $config;
		$encoding = 'utf-8';
		$version = '2.0';
		$title = $page->getProperty('title');
		$text = $page->getProperty('text');
		$request = $engine->getRequest();
		$link = $engine->getUrl($request);

		$this->engine = $engine;
		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			$encoding = $charset;
			$title = '';
		print('<?xml version="1.0" encoding="'
			.$this->escapeAttribute($encoding).'"?>'."\n");
		//begin
		print('<rss version="'.$this->escapeAttribute($version).'"'
		       .' xmlns:atom="http://www.w3.org/2005/Atom">
	<channel>'."\n");
		if($title !== FALSE)
			print("\t\t<title>".$this->escape($title)."</title>\n");
		if($link !== FALSE)
		{
			print("\t\t<link>".$this->escape($link)."</link>\n");
			//FIXME this link can only work with HttpFriendlyEngine
			print("\t\t".'<atom:link href="'
				.$this->escapeAttribute($link).'.rss"'
				.' rel="self"'
				.' type="application/rss+xml"/>'."\n");
		}
		if($text !== FALSE)
			print("\t\t<description>".$this->escape($text)
					."</description>\n");
		$children = $page->getChildren();
		foreach($children as $c)
		{
			if(($type = $c->getType()) != 'row')
				continue;
			print("\t\t<item>\n");
			//properties
			$this->_renderTag($c, 'title');
			$this->_renderTag($c, 'author', 'username');
		       	//XXX format the date as expected
			$this->_renderTag($c, 'pubDate', 'timestamp');
			$this->_renderTagLink($c, 'link');
			$this->_renderTagLink($c, 'guid');
			$this->_renderTag($c, 'description', 'content');
			print("\t\t</item>\n");
		}
		//end
		print("\t</channel>
</rss>\n");
		$this->engine = FALSE;
	}

	private function _renderTag($row, $tag, $property = FALSE)
	{
		if($property === FALSE)
			$property = $tag;
		$text = $row->getProperty($property);
		if($text instanceof PageElement)
			$text = $text->getProperty('text');
		if($text === FALSE)
			return;
		print("\t\t\t<$tag>".$this->escape($text)."</$tag>\n");
	}

	private function _renderTagLink($row, $tag)
	{
		$title = $row->getProperty('title');
		if(!($title instanceof PageElement))
			return FALSE;
		$request = $title->getProperty('request');
		if($request === FALSE)
			return;
		print("\t\t\t<$tag>".$this->engine->getUrl($request, TRUE)
				."</$tag>\n");
	}


	//private
	//properties
	private $engine = FALSE;


	//methods
	//escaping
	private function escape($text)
	{
		return str_replace(array('<', '>'), array('&lt;', '&gt;'),
				$text);
	}


	//escapeAttribute
	private function escapeAttribute($text)
	{
		return htmlspecialchars($text, ENT_COMPAT | ENT_XML1);
	}
}

?>
