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


//HtmlFormat
class HtmlFormat extends FormatElements
{
	//protected
	//properties
	protected $doctype = FALSE;


	//methods
	//essential
	//HtmlFormat::match
	protected function match(&$engine, $type = FALSE)
	{
		switch($type)
		{
			case 'text/html':
				return 10;
			default:
				return 0;
		}
	}


	//HtmlFormat::attach
	protected function attach(&$engine, $type = FALSE)
	{
	}


	//public
	//methods
	//rendering
	//HtmlFormat::render
	public function render(&$engine, $page, $filename = FALSE)
	{
		global $config;
		$this->ids = array();
		$this->tags = array();
		$this->titles = array();
		$this->engine = $engine;
		//FIXME also track tags are properly closed

		if($this->doctype !== FALSE)
			print($this->doctype);
		$this->tagOpen('html');
		$this->renderTabs();
		$this->tagOpen('head');
		$this->_render_title($page);
		$this->_render_theme($page);
		$this->_render_base($page);
		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			$this->renderMeta('Content-Type', 'text/html'
					.'; charset='.$charset);
		if(($location = $page->getProperty('location')) !== FALSE)
			$this->renderMeta('Location', $location);
		if(($refresh = $page->getProperty('refresh')) !== FALSE
				&& is_numeric($refresh))
			$this->renderMeta('Refresh', $refresh);
		$this->renderTabs(-1);
		$this->tagClose('head');
		$this->renderTabs();
		$this->tagOpen('body');
		parent::render($engine, $page, $filename);
		$this->renderTabs(-1);
		$this->tagClose('body');
		$this->renderTabs(-1);
		$this->tagClose('html');
		$this->engine = FALSE;
	}

	private function _render_base($page)
	{
		$request = new Request($this->engine);
		$url = dirname($this->engine->getUrl($request, TRUE)).'/'; //XXX
		$this->renderTabs();
		$this->tag('base', FALSE, FALSE, array('href' => $url));
	}

	private function _render_theme($page)
	{
		global $config;

		if(($theme = $config->getVariable(FALSE, 'theme')) === FALSE)
			return;
		//FIXME emit a (debugging) warning if the theme is not readable?
		$this->renderTabs();
		$this->tag('link', FALSE, FALSE, array('rel' => 'stylesheet',
					'href' => "themes/$theme.css",
					'title' => $theme));
		if(($theme = $config->getVariable('format::html',
					'alternate_themes')) != 1)
			return;
		$this->_render_theme_alternate($page, $theme);
	}

	private function _render_theme_alternate($page, $theme)
	{
		if(($dir = opendir(dirname($_SERVER['SCRIPT_FILENAME'])
				.'/themes')) === FALSE)
			return;
		while(($de = readdir($dir)) !== FALSE)
		{
			if(substr($de, -4, 4) != '.css'
					|| $de == "$theme.css")
				continue;
			$this->renderTabs();
			$this->tag('link', FALSE, FALSE, array(
					'rel' => 'alternate stylesheet',
					'href' => "themes/$de",
					'title' => substr($de, 0, -4)));
		}
		closedir($dir);
	}

	private function _render_title($e)
	{
		global $config;

		if(($title = $e->getProperty('title')) === FALSE)
			$title = $config->getVariable(FALSE, 'title');
		if($title !== FALSE)
		{
			$this->renderTabs();
			$this->tag('title', FALSE, $e->getProperty('id'), FALSE,
					$title);
		}
	}


	//protected
	//methods
	//rendering
	protected function renderBlock($e, $tag = 'div')
	{
		$this->renderTabs();
		$this->tagOpen($tag, $e->getType());
		$this->renderChildren($e);
		$this->renderTabs();
		$this->tagClose($tag);
	}


	protected function renderBox($e, $type = 'vbox')
	{
		$this->renderTabs();
		$this->tagOpen('div', $type, $e->getProperty('id'));
		$children = $e->getChildren();
		foreach($children as $c)
		{
			$this->renderTabs();
			$this->tagOpen('div', 'pack');
			$this->renderElement($c);
			if(count($c->getChildren()) > 1)
				$this->renderTabs();
			$this->tagClose('div');
		}
		$this->renderTabs();
		$this->tagClose('div');
	}


	protected function renderButton($e)
	{
		if(($r = $e->getProperty('request')) !== FALSE)
			$url = $this->engine->getUrl($r, FALSE);
		else
			$url = $e->getProperty('url');
		if(($class = $e->getProperty('class')) === FALSE
				&& ($class = $e->getProperty('stock'))
				!== FALSE)
			$class = "stock16 $class";
		$tag = 'a';
		$args = array();
		switch(($type = $e->getProperty('type')))
		{
			case 'reset':
			case 'submit':
				$tag = 'input';
				$url = FALSE;
				if($class === FALSE)
					$class = "stock16 $type";
				$args['type'] = $type;
				if(($name = $e->getProperty('value')) !== FALSE)
					$args['name'] = $name;
				if(($value = $e->getProperty('text')) !== FALSE)
					$args['value'] = $value;
				$this->tag($tag, $class, $e->getProperty('id'),
						$args);
				break;
			case 'button':
			default:
				$type = 'button';
				if($class !== FALSE)
					$class = "button $class";
				$args['href'] = $url;
				$this->tagOpen($tag, $class,
						$e->getProperty('id'), $args);
				$this->renderChildren($e);
				print($this->escape($e->getProperty('text')));
				$this->tagClose($tag);
				break;
		}
	}


	protected function renderCheckbox($e)
	{
		$this->renderTabs();
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


	protected function renderChildren($e)
	{
		$children = $e->getChildren();
		foreach($children as $c)
			$this->renderElement($c);
	}


	protected function renderDialog($e)
	{
		if(($type = $e->getProperty('type')) === FALSE)
			$type = 'message';
		if(($title = $e->getProperty('title')) === FALSE)
			switch($type)
			{
				case 'error':
					$title = _('Error');
					break;
				case 'question':
					$title = _('Question');
					break;
				case 'warning':
					$title = _('Warning');
					break;
					break;
				case 'info':
				case 'message':
					$title = _('Message');
					break;
			}
		$this->renderTabs();
		$this->tagOpen('div', 'dialog '.$type);
		$title = new PageElement('title', array('stock' => $type,
				'text' => $title));
		$this->renderElement($title);
		$this->renderTabs();
		$this->tagOpen('div', 'message', FALSE, FALSE,
				$e->getProperty('text'));
		$this->tagClose('div');
		$this->renderTabs();
		$this->renderChildren($e);
		$this->tagClose('div');
	}


	protected function renderEntry($e)
	{
		$this->renderTabs();
		$this->tagOpen('div', $e->getType());
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text);
			$this->renderElement($l);
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


	protected function renderFileChooser($e)
	{
		$this->renderTabs();
		$this->tagOpen('div', $e->getType());
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text);
			$this->renderElement($l);
		}
		$name = $e->getProperty('name');
		$type = 'file';
		$this->tag('input', $e->getProperty('class'),
				$e->getProperty('id'),
				array('type' => $type, 'name' => $name));
		$this->tagClose('div');
	}


	protected function renderForm($e)
	{
		$auth = $this->engine->getAuth();

		$this->renderTabs();
		$args = array('action' => 'index.php');
		//XXX look for any file upload field
		foreach($e->getChildren() as $c)
			if($c->getType() == 'filechooser')
			{
				$args['enctype'] = 'multipart/form-data';
				break;
			}
		$method = $e->getProperty('idempotent') ? 'get' : 'post';
		$args['method'] = $method;
		$this->tagOpen('form', $e->getType(), $e->getProperty('id'),
				$args);
		if($method === 'post')
		{
			$token = sha1(uniqid(php_uname(), TRUE));
			if(($tokens = $auth->getVariable($this->engine,
						'tokens')) === FALSE)
				$tokens = array();
			$tokens[$token] = time() + 3600;
			$auth->setVariable($this->engine, 'tokens', $tokens);
			$this->_renderFormHidden('_token', $token);
		}
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			$this->_renderFormHidden('module', $r->getModule());
			$this->_renderFormHidden('action', $r->getAction());
			$this->_renderFormHidden('id', $r->getId());
			if(($args = $r->getParameters()) !== FALSE)
				foreach($args as $k => $v)
					$this->_renderFormHidden($k, $v);
		}
		$this->renderChildren($e);
		$this->renderTabs();
		$this->tagClose('form');
	}

	private function _renderFormHidden($name, $value = FALSE)
	{
		if($value === FALSE)
			return;
		$this->renderTabs();
		$this->tag('input', FALSE, FALSE, array('type' => 'hidden',
					'name' => $name, 'value' => $value));
	}


	protected function renderFrame($e)
	{
		$this->renderTabs();
		$this->tagOpen('div', 'frame');
		if(($title = $e->getProperty('title')) !== FALSE)
		{
			$this->renderTabs();
			$this->tag('div', 'title', FALSE, FALSE, $title);
		}
		$this->renderChildren($e);
		$this->renderTabs();
		$this->tagClose('div');
	}


	protected function renderHbox($e)
	{
		$this->renderBox($e, $e->getType());
	}


	protected function renderIconview($e)
	{
		$e->setProperty('view', 'icons');
		$this->renderTreeview($e);
	}


	protected function renderImage($e)
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


	protected function renderInline($e)
	{
		$text = $e->getProperty('text');
		if($e->getType() !== FALSE)
			$this->tag('span', $e->getType(), $e->getProperty('id'),
					FALSE, $text);
		else if($text !== FALSE)
			print($this->escapeText($text));
	}


	protected function renderLabel($e)
	{
		$tag = 'span';
		$attributes = array();
		if(($for = $e->getProperty('for')) !== FALSE)
		{
			$tag = 'label';
			$attributes['for'] = $for;
		}
		$this->tag($tag, 'label', $e->getProperty('id'),
				$attributes, $e->getProperty('text'));
	}


	protected function renderLink($e)
	{
		$attributes = array();
		if(($a = $e->getProperty('name')) !== FALSE)
			$attributes['name'] = $a;
		if(($r = $e->getProperty('request')) !== FALSE)
			$attributes['href'] = $this->engine->getUrl($r, FALSE);
		else if(($u = $e->getProperty('url')) !== FALSE)
			$attributes['href'] = $u;
		if(($title = $e->getProperty('title')) !== FALSE)
			$attributes['title'] = $title;
		$this->tagOpen('a', FALSE, $e->getProperty('id'), $attributes);
		$this->renderChildren($e);
		if(($stock = $e->getProperty('stock')) !== FALSE)
			$this->tag('img', 'stock16 '.$stock, FALSE,
					array('alt' => ''));
		print($this->escapeText($e->getProperty('text')));
		$this->tagClose('a');
	}


	protected function renderMenu($e, $class = FALSE)
	{
		//FIXME really implement
		if(($children = $e->getChildren()) === FALSE
				|| count($children) == 0)
			return;
		$this->tagOpen('ul', $class);
		foreach($children as $c)
			if($c->getType() == 'menuitem')
				$this->renderMenuitem($c);
		$this->renderTabs(-1);
		$this->tagClose('ul');
	}


	protected function renderMenubar($e)
	{
		$this->renderTabs();
		$this->renderMenu($e, 'menubar');
	}


	protected function renderMenuitem($e)
	{
		$this->renderTabs();
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
		$this->renderMenu($e);
		$this->tagClose('li');
	}


	protected function renderMeta($header, $value)
	{
		$this->renderTabs();
		$this->tag('meta', FALSE, FALSE, array('http-equiv' => $header,
					'content' => $value));
	}


	protected function renderPage($e)
	{
		$this->renderChildren($e, 1);
	}


	protected function renderStatusbar($e)
	{
		$this->renderTabs();
		$this->tag('div', 'statusbar', $e->getProperty('id'), FALSE,
				$e->getProperty('text'));
	}


	protected function renderTabs($more = 0)
	{
		print("\n");
		for($i = 0; $i < count($this->tags) + $more; $i++)
			print("\t");
	}


	protected function renderTextview($e)
	{
		$this->renderTabs();
		$this->tagOpen('div', $e->getType());
		if(($text = $e->getProperty('text')) !== FALSE)
		{
			$l = new PageElement('label');
			$l->setProperty('text', $text);
			$this->renderElement($l);
		}
		if(($value = $e->getProperty('value')) === FALSE)
			$value = '';
		$args = array();
		if(($name = $e->getProperty('name')) !== FALSE)
			$args['name'] = $name;
		$this->tag('textarea', $e->getProperty('class'),
				$e->getProperty('id'), $args, $value);
		$this->tagClose('div');
	}


	protected function renderTitle($e)
	{
		$hcnt = count($this->titles);
		$tcnt = count($this->tags);

		/* XXX this algorithm is a bit ugly but seems to work */
		if($hcnt == 0) /* no title set */
		{
			$cur = 1;
			$this->titles[$cur - 1] = $tcnt;
		}
		else if($this->titles[$hcnt - 1] < $tcnt) /* deeper level */
		{
			$this->titles[$hcnt] = $tcnt;
			$cur = $hcnt;
		}
		else if($this->titles[$hcnt - 1] == $tcnt) /* same level */
			$cur = $hcnt - 1;
		else
		{
			for(; $hcnt > 0; $hcnt = count($this->titles))
			{
				$h = $this->titles[$hcnt - 1];
				if($h <= $tcnt)
					break;
				unset($this->titles[$hcnt - 1]);
			}
			$cur = $hcnt - 1;
		}
		$level = $cur;
		$tag = "h$level";
		if(($class = $e->getProperty('class')) === FALSE
				&& ($class = $e->getProperty('stock'))
				!== FALSE)
			switch($level)
			{
				case 1: $class = "stock48 $class"; break;
				case 2: $class = "stock32 $class"; break;
				case 3: $class = "stock24 $class"; break;
				case 4:
				default:$class = "stock16 $class"; break;
			}
		$this->renderTabs();
		$this->tagOpen($tag, $class, $e->getProperty('id'), FALSE,
				$e->getProperty('text'));
		$this->renderChildren($e);
		$this->tagClose($tag);
	}


	protected function renderToolbar($e)
	{
		$this->renderTabs();
		$this->tagOpen('div', 'toolbar');
		$this->renderChildren($e);
		$this->tagClose('div', 'toolbar');
	}


	protected function renderTreeview($e)
	{
		$auth = $this->engine->getAuth();

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
				$c = array('icon', 'label');
				$e->setProperty('columns', $c);
				break;
			default:
				$view = 'details';
				break;
		}
		$class = "treeview $view";
		$method = $e->getProperty('idempotent') ? 'get' : 'post';
		$this->renderTabs();
		$this->tagOpen('form', $class, FALSE, array(
					'action' => 'index.php',
					'method' => $method));
		if($method === 'post')
		{
			$token = sha1(uniqid(php_uname(), TRUE));
			if(($tokens = $auth->getVariable($this->engine,
						'tokens')) === FALSE)
				$tokens = array();
			$tokens[$token] = time() + 3600;
			$auth->setVariable($this->engine, 'tokens', $tokens);
			$this->_renderTreeviewHidden('_token', $token);
		}
		if(($r = $e->getProperty('request')) !== FALSE)
		{
			//FIXME copied from renderForm()
			$this->_renderTreeviewHidden('module', $r->getModule());
			$this->_renderTreeviewHidden('action', $r->getAction());
			$this->_renderTreeviewHidden('id', $r->getId());
			if(($parameters = $r->getParameters()) !== FALSE)
				foreach($r->getParameters() as $k => $v)
					$this->_renderTreeviewHidden($k, $v);
		}
		$this->_renderTreeviewToolbar($e);
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title');
		$this->renderTabs();
		$this->tagOpen('div', 'table');
		$this->_renderTreeviewHeaders($columns);
		//render rows
		$this->_renderTreeviewRows($e, $columns);
		$this->renderTabs(-1);
		$this->tagClose('div');
		$this->renderTabs(-1);
		$this->tagClose('form');
	}

	private function _renderTreeviewHeaders($columns)
	{
		$this->renderTabs();
		$this->tagOpen('div', 'header');
		$this->tag('span', 'detail', FALSE, FALSE, '');
		foreach($columns as $c)
			$this->tag('span', "detail $c", FALSE, FALSE,
					ucfirst($c));
		$this->tagClose('div');
	}

	private function _renderTreeviewHidden($name, $value = FALSE)
	{
		//FIXME copied from _renderFormHidden()
		if($value === FALSE)
			return;
		$this->renderTabs();
		$this->tag('input', FALSE, FALSE, array('type' => 'hidden',
					'name' => $name, 'value' => $value));
	}

	private function _renderTreeviewRows($e, $columns)
	{
		$id = 1;

		if(($children = $e->getChildren()) === FALSE)
			return;
		foreach($children as $c)
		{
			$this->renderTabs();
			if($c->getType() != 'row')
				continue;
			$this->tagOpen('div', 'row');
			$this->tagOpen('span', 'detail');
			$name = $c->getProperty('id');
			$this->tag('input', FALSE, '_check_'.$id, array(
						'type' => 'checkbox',
						'name' => $name));
			$this->tagClose('span');
			$properties = $c->getProperties();
			//FIXME list in columns' order instead
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

	private function _renderTreeviewToolbar($e)
	{
		if(($children = $e->getChildren()) === FALSE)
			return;
		foreach($children as $c)
			if($c->getType() == 'toolbar')
				$this->renderToolbar($c);
	}


	protected function renderVbox($e)
	{
		$this->renderBox($e, $e->getType());
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
		if(array_pop($this->tags) != $name)
			$this->engine->log('LOG_DEBUG', 'Invalid tag sequence');
	}


	private function tagOpen($name, $class = FALSE, $id = FALSE,
			$attributes = FALSE, $content = FALSE)
	{
		array_push($this->tags, $name);
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
	private $engine = FALSE;
	private $ids;
	private $tags;
	private $titles;
}

?>
