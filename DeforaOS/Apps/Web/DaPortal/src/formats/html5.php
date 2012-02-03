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
class Html5Format extends FormatElements
{
	//protected
	//methods
	//essential
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


	//public
	//methods
	//rendering
	//Html5Format::render
	public function render(&$engine, $page, $filename = FALSE)
	{
		$this->ids = array();
		$this->engine = $engine;
		//FIXME also track tags are properly closed

		parent::render($engine, $page, $filename);
		$this->engine = FALSE;
	}


	//protected
	//methods
	//rendering
	protected function renderBlock($e, $level, $tag = 'div')
	{
		$this->renderTabs($level);
		$this->tagOpen($tag, $e->getType());
		$this->renderChildren($e, $level);
		$this->renderTabs($level);
		$this->tagClose($tag);
	}


	protected function renderBox($e, $level, $type = 'vbox')
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


	protected function renderButton($e, $level)
	{
		if(($r = $e->getProperty('request')) !== FALSE)
			$url = $this->engine->getUrl($r, FALSE);
		else
			$url = $e->getProperty('url');
		if(($class = $e->getProperty('stock')) !== FALSE)
			$class = "stock16 $class";
		switch(($type = $e->getProperty('type')))
		{
			case 'button':
				break;
			case 'reset':
			case 'submit':
				$url = FALSE;
				if($class === FALSE)
					$class = "stock16 $type";
				break;
			default:
				$type = 'button';
				break;
		}
		if($url !== FALSE)
			$this->tagOpen('a', FALSE, FALSE, array(
						'href' => $url));
		$args = array('type' => $type);
		if(($name = $e->getProperty('name')) !== FALSE)
			$args['name'] = $name;
		if(($value = $e->getProperty('value')) !== FALSE)
			$args['value'] = $value;
		$this->tagOpen('button', $class, FALSE, $args);
		$this->renderChildren($e, $level);
		print($this->escape($e->getProperty('text')));
		$this->tagClose('button');
		if($url !== FALSE)
			$this->tagClose('a');
	}


	protected function renderCheckbox($e, $level)
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


	protected function renderChildren($e, $level)
	{
		$children = $e->getChildren();
		foreach($children as $c)
			$this->renderElement($c, $level + 1);
	}


	protected function renderDialog($e, $level)
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


	protected function renderEntry($e, $level)
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


	protected function renderForm($e, $level)
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
			if(($args = $r->getParameters()) !== FALSE)
				foreach($args as $k => $v)
					$this->_renderFormHidden($level + 1, $k,
							$v);
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


	protected function renderFrame($e, $level)
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


	protected function renderHbox($e, $level)
	{
		$this->renderBox($e, $level, $e->getType());
	}


	protected function renderHeading($e, $level)
	{
		//FIXME really track the heading level
		$tag = 'h'.($level - 1);
		$this->renderTabs($level);
		$this->tagOpen($tag, $e->getType(), $e->getProperty('id'),
				FALSE, $e->getProperty('text'));
		$this->renderChildren($e, $level);
		$this->tagClose($tag);
	}


	protected function renderIconview($e, $level)
	{
		$e->setProperty('view', 'icons');
		$this->renderTreeview($e, $level);
	}


	protected function renderImage($e, $level)
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


	protected function renderInline($e, $level)
	{
		$text = $e->getProperty('text');
		if($e->getType() !== FALSE)
			$this->tag('span', $e->getType(), $e->getProperty('id'),
					FALSE, $text);
		else if($text !== FALSE)
			print($this->escapeText($text));
	}


	protected function renderLabel($e, $level)
	{
		$attributes = array();
		if(($for = $e->getProperty('for')) !== FALSE)
			$attributes['for'] = $for;
		$this->tag('label', 'label', $e->getProperty('id'),
				$attributes, $e->getProperty('text'));
	}


	protected function renderLink($e, $level)
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


	protected function renderMenu($e, $level, $class = FALSE)
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


	protected function renderMenubar($e, $level)
	{
		$this->renderTabs($level);
		$this->renderMenu($e, $level, 'menubar');
	}


	protected function renderMenuitem($e, $level)
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
			if(count($attributes))
				$this->tagOpen('a', FALSE,
						$e->getProperty('id'),
						$attributes);
			print($this->escapeText($text));
			if(count($attributes))
				$this->tagClose('a');
		}
		$this->renderMenu($e, $level);
		$this->tagClose('li');
	}


	protected function renderMeta($level, $header, $value)
	{
		$this->renderTabs($level);
		$this->tag('meta', FALSE, FALSE, array('http-equiv' => $header,
					'content' => $value));
	}


	protected function renderPage($e, $level = 0)
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


	protected function renderStatusbar($e, $level)
	{
		$this->renderTabs($level);
		$this->tag('div', 'statusbar', $e->getProperty('id'), FALSE,
				$e->getProperty('text'));
	}


	protected function renderTabs($level)
	{
		print("\n");
		for($i = 0; $i < $level; $i++)
			print("\t");
	}


	protected function renderTheme($page)
	{
		global $config;

		if(($theme = $config->getVariable(FALSE, 'theme')) === FALSE)
			return;
		//FIXME emit a (debugging) warning if the theme is not readable?
		$this->renderTabs(2);
		$this->tag('link', FALSE, FALSE, array('rel' => 'stylesheet',
					'href' => "themes/$theme.css"));
	}


	protected function renderTitle($e)
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


	protected function renderToolbar($e, $level)
	{
		$this->renderTabs($level);
		$this->tagOpen('div', 'toolbar');
		$this->renderChildren($e, $level + 1);
		$this->tagClose('div', 'toolbar');
	}


	protected function renderTreeview($e, $level)
	{
		switch(($view = $e->getProperty('view')))
		{
			case 'details':
			case 'list':
			case 'thumbnails':
			case 'preview':
				break;
			case 'icons':
				if(($c = $e->getProperty('columns')) !== FALSE)
					break;
				$e->setProperty('columns', array('icon',
							'label'));
				break;
			default:
				$view = 'details';
				break;
		}
		$class = "treeview $view";
		$method = $e->getProperty('idempotent') ? 'get' : 'post';
		$this->renderTabs($level);
		$this->tagOpen('form', $class, FALSE, array(
					'action' => 'index.php',
					'method' => $method));
		if($method === 'post' && @session_start())
		{
			//FIXME move this code into the Auth module
			if(!isset($_SESSION['tokens']))
				$_SESSION['tokens'] = array();
			$token = sha1(uniqid(php_uname(), TRUE));
			$_SESSION['tokens'][$token] = time() + 3600;
			$this->_renderTreeviewHidden($level + 1, '_token',
					$token);
		}
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			//FIXME copied from renderForm()
			$this->_renderTreeviewHidden($level + 1, 'module',
					$r->getModule());
			$this->_renderTreeviewHidden($level + 1, 'action',
					$r->getAction());
			$this->_renderTreeviewHidden($level + 1, 'id',
					$r->getId());
			if(($parameters = $r->getParameters()) !== FALSE)
				foreach($r->getParameters() as $k => $v)
					$this->_renderTreeviewHidden($level + 1,
							$k, $v);
		}
		if(($children = $e->getChildren()) !== FALSE)
			foreach($children as $c)
				if($c->getType() == 'toolbar')
					$this->renderToolbar($c, $level + 1);
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title');
		$this->renderTabs($level + 1);
		$this->tagOpen('div', 'table');
		$this->_renderTreeviewHeaders($columns, $level + 1);
		//render rows
		$this->_renderTreeviewRows($e, $columns, $level + 1);
		$this->renderTabs($level + 1);
		$this->tagClose('div');
		$this->renderTabs($level);
		$this->tagClose('form');
	}

	private function _renderTreeviewHeaders($columns, $level)
	{
		$this->renderTabs($level + 1);
		$this->tagOpen('div', 'header');
		$this->tag('span', 'detail', FALSE, FALSE, '');
		foreach($columns as $c)
			$this->tag('span', "detail $c", FALSE, FALSE,
					ucfirst($c));
		$this->tagClose('div');
	}

	private function _renderTreeviewHidden($level, $name, $value = FALSE)
	{
		//FIXME copied from _renderFormHidden()
		if($value === FALSE)
			return;
		$this->renderTabs($level);
		$this->tag('input', FALSE, FALSE, array('type' => 'hidden',
					'name' => $name, 'value' => $value));
	}

	private function _renderTreeviewRows($e, $columns, $level)
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
			$this->tag('input', FALSE, '_check_'.$id, array(
						'type' => 'checkbox',
						'name' => $c->getProperty('id')));
			$this->tagClose('span');
			$properties = $c->getProperties();
			foreach($properties as $k => $v)
			{
				if(!in_array($k, $columns)
						&& !isset($columns[$k]))
					continue;
				$this->tagOpen('span', "detail $k");
				if(is_object($v))
					$this->renderElement($v);
				else
					$this->tag('label', FALSE, FALSE,
							array('for' => '_check_'
								.$id), $v);
				$this->tagClose('span');
			}
			$this->tagClose('div');
			$id++;
		}
	}


	protected function renderVbox($e, $level)
	{
		$this->renderBox($e, $level, $e->getType());
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
		$from = array('<', '>', "\n", "\r");
		$to = array('&lt;', '&gt;', "<br />\n", '');

		return str_replace($from, $to, $text);
	}


	private function escapeURI($text)
	{
		return urlencode($text);
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
