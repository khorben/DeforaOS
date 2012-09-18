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
	protected function match($engine, $type = FALSE)
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
	protected function attach($engine, $type = FALSE)
	{
	}


	//public
	//methods
	//rendering
	//PlainFormat::render
	public function render($engine, $page, $filename = FALSE)
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
	//printing
	//PlainFormat::print
	protected function _print($string)
	{
		print($string);
	}


	//rendering
	//PlainFormat::renderBlock
	protected function renderBlock($e, $underline = '-')
	{
		if($this->separator != '')
			$this->_print("\n\n");
		if(($title = $e->getProperty('title')) !== FALSE)
		{
			$this->_print("$title\n");
			for($i = 0; $i < strlen($title); $i++)
				$this->_print($underline);
			$this->_print("\n\n");
		}
		$this->separator = '';
		$this->renderInline($e);
		$this->_print("\n\n");
	}


	//PlainFormat::renderChildren
	protected function renderChildren($e)
	{
		if(($children = $e->getChildren()) === FALSE)
			return;
		foreach($children as $c)
			$this->renderElement($c);
	}


	//PlainFormat::renderElement
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


	//PlainFormat::renderInline
	protected function renderInline($e)
	{
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$this->_print($this->separator.$text);
			$this->separator = ' ';
		}
		$this->renderChildren($e);
	}


	//PlainFormat::renderLink
	protected function renderLink($e)
	{
		if(($url = $e->getProperty('url')) === FALSE
				&& ($r = $e->getProperty('request')) !== FALSE)
			$url = $this->engine->getUrl($r);
		if(($text = $e->getProperty('text')) !== FALSE
				&& strlen($text) > 0)
		{
			$this->_print($this->separator.$text);
			$this->separator = ' ';
		}
		if($url !== FALSE)
		{
			$this->_print($this->separator."($url)");
			$this->separator = ' ';
		}
		$this->renderChildren($e);
	}
}

?>
