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


//PlainFormat
class PlainFormat extends Format
{
	//protected
	//properties
	protected $engine = FALSE;
	protected $separator = '';


	//methods
	//essential
	//PlainFormat::match
	protected function match(&$engine, $type = FALSE)
	{
		switch($type)
		{
			case 'text/plain':
				return 100;
			default:
				return 0;
		}
	}


	//PlainFormat::attach
	protected function attach(&$engine, $type = FALSE)
	{
	}


	//public
	//methods
	//rendering
	public function render(&$engine, $page, $filename = FALSE)
	{
		//FIXME ignore filename for the moment
		if($page === FALSE)
			$page = new Page;
		$this->engine = $engine;
		$this->renderElement($page);
		$this->engine = FALSE;
	}


	//protected
	//methods
	//rendering
	protected function renderBlock($e, $underline = '-')
	{
		if($this->separator != '')
			print("\n\n");
		if(($title = $e->getProperty('title')) !== FALSE)
		{
			print("$title\n");
			for($i = 0; $i < strlen($title); $i++)
				print($underline);
			print("\n\n");
		}
		$this->separator = '';
		$this->renderInline($e);
		print("\n\n");
	}


	protected function renderChildren($e)
	{
		if(($children = $e->getChildren()) === FALSE)
			return;
		foreach($children as $c)
			$this->renderElement($c);
	}


	protected function renderElement($e)
	{
		switch($e->getType())
		{
			case 'dialog':
			case 'frame':
			case 'hbox':
			case 'menubar':
			case 'statusbar':
			case 'vbox':
				return $this->renderBlock($e);
			case 'page':
				return $this->renderBlock($e, '=');
			case 'link':
				return $this->renderLink($e);
			case 'label':
			default:
				return $this->renderInline($e);
		}
	}


	protected function renderInline($e)
	{
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			print($this->separator.$text);
			$this->separator = ' ';
		}
		$this->renderChildren($e);
	}


	protected function renderLink($e)
	{
		if(($url = $e->getProperty('url')) === FALSE
				&& ($r = $e->getProperty('request')) !== FALSE)
			$url = $this->engine->getUrl($r);
		if(($text = $e->getProperty('text')) !== FALSE
				&& strlen($text) > 0)
		{
			print($this->separator.$text);
			$this->separator = ' ';
		}
		if($url !== FALSE)
		{
			print($this->separator."($url)");
			$this->separator = ' ';
		}
		$this->renderChildren($e);
	}
}

?>
