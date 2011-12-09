<?php //$Id$
//Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>
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



//HtmlTemplate
class HtmlTemplate extends Template
{
	public function render(&$engine, $page)
	{
		global $config;

		$this->engine = $engine;
		print("<!DOCTYPE html>\n<html>\n\t<head>\n");
		$this->renderTitle($page);
		$this->renderTheme($page);
		print("\t</head>\n\t<body>");
		if(($top = $engine->process(new Request($engine, 'top')))
				!== FALSE)
			$this->renderElement($top);
		$this->renderChildren($page, 1);
		$this->renderTabs(1);
		print("</body>\n</html>\n");
		$this->engine = FALSE;
	}

	private function renderBlock($e, $level, $tag = 'div')
	{
		$this->renderTabs($level);
		print('<'.$tag.' class="'.$this->escapeAttribute($e->getType())
				.'">'."\n");
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		print("</$tag>\n");
	}

	private function renderBox($e, $level, $type = 'vbox')
	{
		$this->renderTabs($level);
		print('<div class="'.$this->escapeAttribute($type).'">');
		$children = $e->getChildren();
		foreach($children as $c)
		{
			$this->renderTabs($level + 1);
			print('<div class="pack">');
			$this->renderElement($c, $level + 2);
			print('</div>');
		}
		$this->renderTabs($level);
		print("</div>\n");
	}

	private function renderButton($e, $level)
	{
		$type = 'button';
		$class = $e->getProperty('class');
		switch($e->getProperty('type'))
		{
			case 'button':
				break;
			case 'reset':
			case 'submit':
				$type = $e->getProperty('type');
				if($class === FALSE)
					$class = $type;
				break;
			default:
				$type = 'button';
				break;
		}
		print('<button type="'.$type.'"');
		if($class !== FALSE)
			print(' class="'.$this->escapeAttribute($class).'"');
		print('>');
		$this->renderChildren($e, $level);
		print($this->escape($e->getProperty('text')));
		print('</button>');
	}

	private function renderCheckbox($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="'.$this->escapeAttribute($e->getType())
				.'">');
		print('<input type="checkbox"');
		if(($name = $e->getProperty('name')) != FALSE)
		{
			//FIXME the ID may not be unique
			print(' id="'.$this->escapeAttribute($name).'"');
			print(' name="'.$this->escapeAttribute($name).'"');
		}
		if(($value = $e->getProperty('value')) !== FALSE)
			print(' checked="checked"');
		print('/>');
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text);
			if($name !== FALSE)
				$l->setProperty('for', $name);
			$this->renderElement($l, $text);
		}
		print('</div>');
	}

	private function renderChildren($e, $level)
	{
		$children = $e->getChildren();
		foreach($children as $c)
			$this->renderElement($c, $level + 1);
	}

	private function renderDialog($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="dialog">');
		$this->renderTabs($level + 1);
		print('<div class="title">'.$this->escape(
					$e->getProperty('title')).'</div>');
		$this->renderTabs($level + 1);
		print('<div class="message">'.$this->escape(
					$e->getProperty('text')).'</div>');
		$this->renderTabs($level + 1);
		$this->renderChildren($e, $level);
		print('</div>');
	}

	private function renderElement($e, $level = 1)
	{
		switch(($type = $e->getType()))
		{
			case 'button':
				return $this->renderButton($e, $level);
			case 'checkbox':
				return $this->renderCheckbox($e, $level);
			case 'dialog':
				return $this->renderDialog($e, $level);
			case 'entry':
				return $this->renderEntry($e, $level);
			case 'form':
				return $this->renderForm($e, $level);
			case 'frame':
				return $this->renderFrame($e, $level);
			case 'hbox':
			case 'vbox':
				return $this->renderBox($e, $level, $type);
			case 'label':
				return $this->renderLabel($e, $level);
			case 'link':
				return $this->renderLink($e, $level);
			case 'menubar':
				return $this->renderMenubar($e, $level);
			case 'page':
				return $this->renderBlock($e, $level);
			case 'row':
				return $this->renderBlock($e, $level);
			case 'statusbar':
				return $this->renderStatusbar($e, $level);
			case 'title':
				return $this->renderHeading($e, $level);
			case 'treeview':
			case 'iconview': /* XXX really implement */
				return $this->renderTreeview($e, $level);
			default:
				return $this->renderInline($e, $level);
		}
	}

	private function renderEntry($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="'.$this->escapeAttribute($e->getType())
				.'">');
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text.': ');
			$this->renderElement($l, $level);
		}
		$name = $e->getProperty('name');
		$value = $e->getProperty('value');
		$type = ($e->getProperty('hidden') === TRUE) ? 'password'
			: 'text';
		print('<input type="'.$type.'"'
				.' name="'.$this->escapeAttribute($name).'"'
				.' value="'.$this->escapeAttribute($value).'"'
				.'/>');
		print('</div>');
	}

	private function renderForm($e, $level)
	{
		$this->renderTabs($level);
		print('<form class="'.$this->escapeAttribute($e->getType()).'"'
				.'>');
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			$this->renderFormHidden($level + 1, 'module',
					$r->getModule());
			$this->renderFormHidden($level + 1, 'action',
					$r->getAction());
			$this->renderFormHidden($level + 1, 'id', $r->getId());
		}
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		print("</form>");
	}

	private function renderFormHidden($level, $name, $value = FALSE)
	{
		if($value === FALSE)
			return;
		$this->renderTabs($level);
		print('<input type="hidden"'
				.' name="'.$this->escapeAttribute($name).'"'
				.' value="'.$this->escapeAttribute($value).'"'
				.'/>');
	}

	private function renderFrame($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="frame">');
		if(($title = $e->getProperty('title')) !== FALSE)
		{
			$this->renderTabs($level + 1);
			print('<div class="title">'.$this->escape($title)
			.'</div>');
		}
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		print('</div>');
	}

	private function renderHeading($e, $level)
	{
		$tag = 'h'.($level - 1);
		$this->renderTabs($level);
		print('<'.$tag.' class="'.$this->escapeAttribute($e->getType())
				.'">'.$this->escape($e->getProperty('text'))
				. "</$tag>");
	}

	private function renderInline($e, $level)
	{
		print('<span class="'.$this->escapeAttribute($e->getType())
				.'">');
		if(($text = $e->getProperty('text')) !== FALSE)
			print($this->escape($text));
		print("</span>");
	}

	private function renderLabel($e, $level)
	{
		if(($for = $e->getProperty('for')) === FALSE)
			return $this->renderInline($e, $level);
		print('<label class="label"'
				.' for="'.$this->escapeAttribute($for).'">');
		if(($text = $e->getProperty('text')) !== FALSE)
			print($this->escape($text));
		print("</label>");
	}

	private function renderLink($e, $level)
	{
		print('<a');
		if(($a = $e->getProperty('name')) !== FALSE)
			print(' name="'.$this->escapeAttribute($a).'"');
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			//FIXME properly output arguments
			print(' href="index.php');
			$sep = $this->escapeAttribute('?');
			if(($m = $r->getModule()) !== FALSE)
			{
				print($sep.'module='.$this->escapeURI($m));
				$sep = $this->escapeAttribute('&');
			}
			if(($a = $r->getAction()) !== FALSE)
			{
				print($sep.'action='.$this->escapeURI($a));
				$sep = $this->escapeAttribute('&');
			}
			print('"');
		}
		print('>');
		print($this->escape($e->getProperty('text')));
		print("</a>");
	}

	private function renderMenu($e, $level)
	{
		//FIXME really implement
		$children = $e->getChildren();
		foreach($children as $c)
			if($c->getType() == 'menuitem')
				$this->renderMenuitem($c, $level);
	}

	private function renderMenubar($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="menubar">');
		$this->renderMenu($e, $level + 1);
		$this->renderTabs($level);
		print('</div>');
	}

	private function renderMenuitem($e, $level)
	{
		//FIXME really implement
		$this->renderTabs($level);
		print('<div class="menuitem">');
		if(($text = $e->getProperty('text')) !== FALSE)
			print($this->escape($text));
		print('</div>');
	}

	private function renderStatusbar($e, $level)
	{
		$this->renderTabs($level);
		print('<div class="statusbar">');
		if(($text = $e->getProperty('text')) !== FALSE)
			print($this->escape($text));
		$this->renderTabs($level);
		print("</div>");
	}

	private function renderTabs($level)
	{
		print("\n");
		for($i = 0; $i < $level; $i++)
			print("\t");
	}

	private function renderTheme($page)
	{
		global $config;

		if(($theme = $config->getVariable(FALSE, 'theme')) === FALSE)
			return;
		print("\t\t".'<link rel="stylesheet" href="themes/'
				.$this->escapeAttribute($theme).'.css"/>'."\n");
	}

	private function renderTitle($page)
	{
		global $config;

		if(($title = $page->getProperty('title')) === FALSE)
			$title = $config->getVariable(FALSE, 'title');
		if($title !== FALSE)
			print("\t\t<title>".$this->escape($title)."</title>\n");
	}

	private function renderTreeview($e, $level)
	{
		$this->renderTabs($level);
		switch(($view = $e->getProperty('view')))
		{
			case 'details':
			case 'icons':
			case 'list':
			case 'thumbnails':
			case 'preview':
				break;
			default:
				$view = 'details';
				break;
		}
		$class = $e->getType()." $view";
		print('<form class="'.$this->escapeAttribute($class).'">');
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title');
		$this->renderTreeviewHeaders($columns, $level);
		//render rows
		$this->renderTreeviewRows($e, $columns, $level);
		$this->renderTabs($level);
		print('</form>');
	}

	private function renderTreeviewHeaders($columns, $level)
	{
		$this->renderTabs($level + 1);
		print('<div class="header">');
		foreach($columns as $c)
			print('<span class="detail '.$this->escapeAttribute($c)
					.'">'.$this->escape(ucfirst($c))
					.'</span>');
		print('</div>');
	}

	private function renderTreeviewRows($e, $columns, $level)
	{
		$children = $e->getChildren();
		foreach($children as $c)
		{
			$this->renderTabs($level + 1);
			if($c->getType() != 'row')
				continue;
			print('<div class="row">');
			$properties = $c->getProperties();
			foreach($properties as $k => $v)
			{
				print('<span class="');
				if(in_array($k, $columns))
					print('detail ');
				print($this->escapeAttribute($k).'">');
				$this->renderChildren($c, $level);
				print($this->escape($v));
				print('</span>');
			}
			print('</div>');
		}
	}


	private function escape($text)
	{
		return str_replace(array('<', '>'), array('&lt;', '&gt;'),
				$text);
	}


	private function escapeAttribute($text)
	{
		return htmlspecialchars($text);
	}


	private function escapeURI($text)
	{
		return urlencode($text);
	}


	//protected
	protected function attach(&$backend)
	{
		return TRUE;
	}


	protected function match(&$backend)
	{
		switch($backend->getType())
		{
			case 'text/html':
				return 100;
			default:
				return 0;
		}
	}


	//private
	private $engine = FALSE;
}

?>
