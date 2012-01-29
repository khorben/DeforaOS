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


//Html5Format
class Html5Format extends Format
{
	//protected
	//methods
	//Html5Format::match
	protected function match(&$engine, $type)
	{
		switch($type)
		{
			case 'text/html':
				return 100;
			default:
				return 0;
		}
	}


	//Html5Format::attach
	protected function attach(&$engine, $type)
	{
	}


	//Html5Format::render
	public function render(&$engine, $page, $filename = FALSE)
	{
		$this->ids = array();
		$this->engine = $engine;
		//FIXME also track tags are properly closed

		//FIXME ignore $filename for the moment
		if($page === FALSE)
		{
			$p = new Page();
			$this->renderElement($p);
		}
		if($page->getType() == 'page')
			$this->renderElement($page);
		else
		{
			$title = $page->getProperties('title');
			$p = new Page(array('title' => $title));
			$p->appendElement($page);
			$this->renderElement($p);
		}
		$this->engine = FALSE;
	}


	//private
	//methods
	//escaping
	private function escape($text)
	{
		return str_replace(array('<', '>'), array('&lt;', '&gt;'),
				$text);
	}


	private function escapeAttribute($text)
	{
		return htmlspecialchars($text);
	}


	private function escapeText($text)
	{
		$from = array('<', '>', "\n");
		$to = array('&lt;', '&gt;', '<br />');

		return str_replace($from, $to, $text);
	}


	private function escapeURI($text)
	{
		return urlencode($text);
	}


	//rendering
	private function renderBlock($e, $level, $tag = 'div')
	{
		$this->renderTabs($level);
		$this->tagOpen($tag, $e->getType());
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		$this->tagClose($tag);
	}


	private function renderBox($e, $level, $type = 'vbox')
	{
		$this->renderTabs($level);
		$this->tagOpen('div', $type);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			$this->renderTabs($level + 1);
			$this->tagOpen('div', 'pack');
			$this->renderElement($c, $level + 2);
			if(count($c->getChildren()) > 1)
				$this->renderTabs($level + 1);
			$this->tagClose('div');
		}
		$this->renderTabs($level);
		$this->tagClose('div');
	}


	private function renderButton($e, $level)
	{
		if(($r = $e->getProperty('request')) !== FALSE)
			$url = $this->engine->getUrl($r, FALSE);
		else
			$url = $e->getProperty('url');
		if($url !== FALSE)
			$this->tagOpen('a', FALSE, FALSE, array(
						'href' => $url));
		if(($class = $e->getProperty('stock')) !== FALSE)
			$class = "stock16 $class";
		switch(($type = $e->getProperty('type')))
		{
			case 'button':
				break;
			case 'reset':
			case 'submit':
				$type = $e->getProperty('type');
				if($class === FALSE)
					$class = "stock16 $type";
				break;
			default:
				$type = 'button';
				break;
		}
		$this->tagOpen('button', $class, FALSE, array('type' => $type));
		$this->renderChildren($e, $level);
		print($this->escape($e->getProperty('text')));
		$this->tagClose('button');
		if($url !== FALSE)
			$this->tagClose('a');
	}


	private function renderCheckbox($e, $level)
	{
		$this->renderTabs($level);
		$this->tagOpen('div', $e->getType());
		print('<input type="checkbox"');
		if(($name = $e->getProperty('name')) !== FALSE)
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
		$this->tagClose('div');
	}


	private function renderChildren($e, $level)
	{
		$children = $e->getChildren();
		foreach($children as $c)
			$this->renderElement($c, $level + 1);
	}


	private function renderDialog($e, $level)
	{
		if(($type = $e->getProperty('type')) === FALSE)
			$type = 'message';
		if(($title = $e->getProperty('title')) === FALSE)
			switch($type)
			{
				case 'error':
				case 'question':
				case 'warning':
					$title = ucfirst($type);
					break;
				case 'info':
				case 'message':
					$title = 'Message';
					break;
			}
		$this->renderTabs($level);
		$this->tagOpen('div', 'dialog '.$type);
		$this->renderTabs($level + 1);
		$box = new PageElement('hbox');
		$box->append('image', array('stock' => $type));
		$box->append('title', array('text' => $title));
		$this->renderElement($box, $level + 1);
		$this->renderTabs($level + 1);
		$this->tagOpen('div', 'message', FALSE, FALSE,
				$e->getProperty('text'));
		$this->tagClose('div');
		$this->renderTabs($level);
		$this->renderChildren($e, $level);
		$this->tagClose('div');
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
			case 'image':
				return $this->renderImage($e, $level);
			case 'label':
				return $this->renderLabel($e, $level);
			case 'link':
				return $this->renderLink($e, $level);
			case 'menubar':
				return $this->renderMenubar($e, $level);
			case 'page':
				return $this->renderPage($e, $level);
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
		$this->tagOpen('div', $e->getType());
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text);
			$this->renderElement($l, $level);
		}
		$name = $e->getProperty('name');
		$value = $e->getProperty('value');
		$type = ($e->getProperty('hidden') === TRUE) ? 'password'
			: 'text';
		$this->tag('input', $e->getProperty('class'),
				$e->getProperty('id'),
				array('type' => $type,
					'name' => $name, 'value' => $value));
		$this->tagClose('div');
	}


	private function renderForm($e, $level)
	{
		$this->renderTabs($level);
		$method = $e->getProperty('idempotent') ? 'get' : 'post';
		$this->tagOpen('form', $e->getType(), $e->getProperty('id'),
				array('action' => 'index.php',
					'method' => $method));
		if($method === 'post' && @session_start())
		{
			//FIXME move this code into the Auth module
			if(!isset($_SESSION['tokens']))
				$_SESSION['tokens'] = array();
			$token = sha1(uniqid(php_uname(), TRUE));
			$_SESSION['tokens'][$token] = time() + 3600;
			$this->_renderFormHidden($level + 1, '_token', $token);
		}
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			$this->_renderFormHidden($level + 1, 'module',
					$r->getModule());
			$this->_renderFormHidden($level + 1, 'action',
					$r->getAction());
			$this->_renderFormHidden($level + 1, 'id', $r->getId());
			foreach($r->getParameters() as $k => $v)
				$this->_renderFormHidden($level + 1, $k, $v);
		}
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		$this->tagClose('form');
	}

	private function _renderFormHidden($level, $name, $value = FALSE)
	{
		if($value === FALSE)
			return;
		$this->renderTabs($level);
		$this->tag('input', FALSE, FALSE, array('type' => 'hidden',
					'name' => $name, 'value' => $value));
	}


	private function renderFrame($e, $level)
	{
		$this->renderTabs($level);
		$this->tagOpen('div', 'frame');
		if(($title = $e->getProperty('title')) !== FALSE)
		{
			$this->renderTabs($level + 1);
			$this->tag('div', 'title', FALSE, FALSE, $title);
		}
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		$this->tagClose('div');
	}


	private function renderHeading($e, $level)
	{
		//FIXME really track the heading level
		$tag = 'h'.($level - 1);
		$this->renderTabs($level);
		$this->tagOpen($tag, $e->getType(), $e->getProperty('id'),
				FALSE, $e->getProperty('text'));
		$this->renderChildren($e, $level);
		$this->tagClose($tag);
	}


	private function renderImage($e, $level)
	{
		$size = 48;
		$class = FALSE;
		$attributes = array('alt' => $e->getProperty('text'));

		if(($title = $e->getProperty('title')) !== FALSE)
			$attributes['title'] = $title;
		if(($stock = $e->getProperty('stock')) !== FALSE)
		{
			if(($s = $e->getProperty('size')) !== FALSE
					&& is_numeric($s))
				$size = $s;
			$class = "stock$size $stock";
		}
		else
			$attributes['src'] = $e->getProperty('source');
		$this->tag('img', $class, $e->getProperty('id'), $attributes);
	}


	private function renderInline($e, $level)
	{
		$text = $e->getProperty('text');
		if($e->getType() !== FALSE)
			$this->tag('span', $e->getType(), $e->getProperty('id'),
					FALSE, $text);
		else if($text !== FALSE)
			print($this->escapeText($text));
	}


	private function renderLabel($e, $level)
	{
		$attributes = array();
		if(($for = $e->getProperty('for')) !== FALSE)
			$attributes['for'] = $for;
		$this->tag('label', 'label', $e->getProperty('id'),
				$attributes, $e->getProperty('text'));
	}


	private function renderLink($e, $level)
	{
		$attributes = array();
		if(($a = $e->getProperty('name')) !== FALSE)
			$attributes['name'] = $a;
		if(($r = $e->getProperty('request')) !== FALSE)
			$attributes['href'] = $this->engine->getUrl($r, FALSE);
		else if(($u = $e->getProperty('url')) !== FALSE)
			$attributes['href'] = $u;
		$this->tagOpen('a', FALSE, $e->getProperty('id'), $attributes);
		$this->renderChildren($e, $level);
		print($this->escapeText($e->getProperty('text')));
		$this->tagClose('a');
	}


	private function renderMenu($e, $level, $class = FALSE)
	{
		//FIXME really implement
		if(($children = $e->getChildren()) === FALSE
				|| count($children) == 0)
			return;
		$this->tagOpen('ul', $class);
		foreach($children as $c)
			if($c->getType() == 'menuitem')
				$this->renderMenuitem($c, $level + 1);
		$this->renderTabs($level);
		$this->tagClose('ul');
	}


	private function renderMenubar($e, $level)
	{
		$this->renderTabs($level);
		$this->renderMenu($e, $level, 'menubar');
	}


	private function renderMenuitem($e, $level)
	{
		$this->renderTabs($level);
		$this->tagOpen('li', 'menuitem');
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$attributes = array();
			if(($a = $e->getProperty('name')) !== FALSE)
				$attributes['name'] = $a;
			if(($r = $e->getProperty('request')) !== FALSE)
				$attributes['href'] = $this->engine->getUrl($r,
						FALSE);
			else if(($u = $e->getProperty('url')) !== FALSE)
				$attributes['href'] = $u;
			$this->tagOpen('a', FALSE, $e->getProperty('id'),
					$attributes);
			print($this->escapeText($text));
			$this->tagClose('a');
		}
		$this->renderMenu($e, $level);
		$this->tagClose('li');
	}


	private function renderMeta($level, $header, $value)
	{
		$this->renderTabs($level);
		$this->tag('meta', FALSE, FALSE, array('http-equiv' => $header,
					'content' => $value));
	}


	private function renderPage($e)
	{
		global $config;

		print("<!DOCTYPE html>\n");
		$this->tagOpen('html');
		$this->renderTabs(1);
		$this->tagOpen('head');
		$this->renderTitle($e);
		$this->renderTheme($e);
		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			$this->renderMeta(2, 'Content-Type', 'text/html'
					.'; charset='.$charset);
		if(($location = $e->getProperty('location')) !== FALSE)
			$this->renderMeta(2, 'Location', $location);
		if(($refresh = $e->getProperty('refresh')) !== FALSE
				&& is_numeric($refresh))
			$this->renderMeta(2, 'Refresh', $refresh);
		$this->renderTabs(1);
		$this->tagClose('head');
		$this->renderTabs(1);
		$this->tagOpen('body');
		$this->renderChildren($e, 1);
		$this->renderTabs(1);
		$this->tagClose('body');
		$this->renderTabs(0);
		$this->tagClose('html');
	}


	private function renderStatusbar($e, $level)
	{
		$this->renderTabs($level);
		$this->tag('div', 'statusbar', $e->getProperty('id'), FALSE,
				$e->getProperty('text'));
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
		//FIXME emit a (debugging) warning if the theme is not readable?
		$this->renderTabs(2);
		$this->tag('link', FALSE, FALSE, array('rel' => 'stylesheet',
					'href' => "themes/$theme.css"));
	}


	private function renderTitle($e)
	{
		global $config;

		if(($title = $e->getProperty('title')) === FALSE)
			$title = $config->getVariable(FALSE, 'title');
		if($title !== FALSE)
		{
			$this->renderTabs(2);
			$this->tag('title', FALSE, $e->getProperty('id'), FALSE,
					$title);
		}
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
				if($e->getType() == 'iconview')
					$view = 'icons';
				else
					$view = 'details';
				break;
		}
		$class = "treeview $view";
		$method = $e->getProperty('idempotent') ? 'get' : 'post';
		$this->tagOpen('form', $class, FALSE, array(
					'method' => $method));
		//FIXME protect against CSRF attacks
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title');
		$this->renderTreeviewHeaders($columns, $level);
		//render rows
		$this->renderTreeviewRows($e, $columns, $level);
		$this->renderTabs($level);
		$this->tagClose('form');
	}


	private function renderTreeviewHeaders($columns, $level)
	{
		$this->renderTabs($level + 1);
		$this->tagOpen('div', 'header');
		$this->tag('span', 'detail', FALSE, FALSE, '');
		foreach($columns as $c)
			$this->tag('span', "detail $c", FALSE, FALSE,
					ucfirst($c));
		$this->tagClose('div');
	}


	private function renderTreeviewRows($e, $columns, $level)
	{
		$id = 1;

		$children = $e->getChildren();
		foreach($children as $c)
		{
			$this->renderTabs($level + 1);
			if($c->getType() != 'row')
				continue;
			$this->tagOpen('div', 'row');
			$this->tagOpen('span', 'detail');
			$this->tag('input', FALSE, 'check_'.$id, array(
						'type' => 'checkbox'));
			$this->tagClose('span');
			$properties = $c->getProperties();
			foreach($properties as $k => $v)
			{
				print('<span class="');
				if(in_array($k, $columns))
					print('detail ');
				print($this->escapeAttribute($k).'">');
				if(is_string($v))
					$this->tagOpen('label', FALSE, FALSE,
							array('for' => 'check_'.$id), $v);
				else
					$this->renderElement($v);
				$this->tagClose('span');
			}
			$this->tagClose('div');
			$id++;
		}
	}


	//tagging
	private function tag($name, $class = FALSE, $id = FALSE,
			$attributes = FALSE, $content = FALSE)
	{
		//FIXME automatically output tabs
		$tag = '<'.$this->escapeAttribute($name);
		if($class !== FALSE)
			$tag.=' class="'.$this->escapeAttribute($class).'"';
		if($id !== FALSE)
		{
			if(isset($ids[$id]))
				$engine->log('LOG_DEBUG', 'HTML ID '.$id
						.' is already defined');
			$ids[$id] = TRUE;
			$tag.=' id="'.$this->escapeAttribute($id).'"';
		}
		if(is_array($attributes))
			foreach($attributes as $k => $v)
				$tag.=' '.$this->escapeAttribute($k).'="'
				.$this->escapeAttribute($v).'"';
		if($content !== FALSE)
			$tag.='>'.$this->escapeText($content)
				.'</'.$this->escapeAttribute($name).'>';
		else
			$tag.='/>';
		print($tag);
	}


	private function tagClose($name)
	{
		print('</'.$this->escapeAttribute($name).'>');
	}


	private function tagOpen($name, $class = FALSE, $id = FALSE,
			$attributes = FALSE, $content = FALSE)
	{
		//FIXME automatically output tabs
		$tag = '<'.$this->escapeAttribute($name);
		if($class !== FALSE)
			$tag.=' class="'.$this->escapeAttribute($class).'"';
		if($id !== FALSE)
		{
			if(isset($ids[$id]))
				$engine->log('LOG_DEBUG', 'HTML ID '.$id
						.' is already defined');
			$ids[$id] = TRUE;
			$tag.=' id="'.$this->escapeAttribute($id).'"';
		}
		if(is_array($attributes))
			foreach($attributes as $k => $v)
				$tag.=' '.$this->escapeAttribute($k).'="'
				.$this->escapeAttribute($v).'"';
		$tag.='>';
		print($tag);
		if($content !== FALSE)
			print($this->escapeText($content));
	}


	//properties
	private $ids;
	private $engine = FALSE;
}

?>
