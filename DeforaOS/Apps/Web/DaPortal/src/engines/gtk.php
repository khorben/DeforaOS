<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
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



require_once('./engines/cli.php');
require_once('./system/format.php');
require_once('./system/page.php');
require_once('./system/template.php');


//GtkEngine
class GtkEngine extends CliEngine
{
	//protected
	//properties
	private $format = FALSE;


	//public
	//methods
	//useful
	//GtkEngine::render
	public function render($page)
	{
		global $config;

		if(($template = Template::attachDefault($this)) === FALSE)
			return FALSE;
		if(($page = $template->render($this, $page)) === FALSE)
			return FALSE;
		$this->format->render($this, $page);
	}


	//GtkEngine::match
	public function match()
	{
		if(($score = parent::match()) != 100)
			return $score;
		if(getenv('DISPLAY') === FALSE)
			return 0;
		if(class_exists('gtk'))
			return $score + 1;
		return 0;
	}


	//GtkEngine::attach
	public function attach()
	{
		parent::attach();
		$this->format = new GtkFormat;
		$this->format->attach($this);
	}


	//GtkEngine::log
	public function log($priority, $message)
	{
		return $this->format->log($priority, $message);
	}
}


class GtkFormat extends FormatElements
{
	//private
	//properties
	private $engine = FALSE;
	private $windows = array();
	private $debug_window = FALSE;
	private $debug_store;

	private $fontTitle;
	private $fontLink;


	//public
	//methods
	//essential
	//GtkFormat::match
	public function match(&$engine, $type = FALSE)
	{
		//never match by default
		return 0;
	}


	//GtkFormat::attach
	public function attach(&$engine, $type = FALSE)
	{
		//initialize fonts
		$this->engine = $engine;
		$this->fontTitle = new PangoFontDescription;
		$this->fontTitle->set_weight('PANGO_WEIGHT_BOLD');
		$this->fontLink = new PangoFontDescription;
	}


	//rendering
	//GtkFormat::render
	public function render(&$engine, $page, $filename = FALSE)
	{
		if($filename !== FALSE)
			return $engine->log('LOG_ERR',
					'Cannot render to a filename');
		parent::render($engine, $page, $filename);
		//enter the main loop
		Gtk::main();
		foreach($this->windows as $w)
			$w->destroy();
		return FALSE;
	}


	//logging
	public function log($priority, $message)
	{
		$icon = Gtk::STOCK_DIALOG_INFO;
		$title = 'Information';
		switch($priority)
		{
			case 'LOG_DEBUG':
				if(!$this->engine->getDebug())
					return FALSE;
				break;
			case 'LOG_ERR':
				$icon = Gtk::STOCK_DIALOG_ERROR;
				$title = 'Error';
				break;
			case 'LOG_WARNING':
				$icon = Gtk::STOCK_DIALOG_WARNING;
				$title = 'Warning';
				break;
		}
		if($this->engine->getDebug())
			$this->_log_append($icon, $priority, $message);
		if($priority == 'LOG_DEBUG')
			return FALSE;
		return FALSE;
	}

	private function _log_append($icon, $priority, $message)
	{
		$theme = GtkIconTheme::get_default();
		$pixbuf = $theme->load_icon($icon, 24, 0);
		if($this->debug_window === FALSE)
			$this->_log_create();
		$iter = $this->debug_store->append();
		$this->debug_store->set($iter, 0, $pixbuf, 1, $priority,
				2, $message);
		$this->debug_window->show_all();
	}

	private function _log_create()
	{
		$this->debug_window = new GtkWindow();
		$this->debug_window->set_default_size(400, 200);
		$this->debug_window->set_title('Debugging console');
		$this->debug_window->connect_simple('delete-event',
				array($this, '_log_on_delete_event'),
				$this->debug_window);
		$widget = new GtkScrolledWindow();
		$widget->set_policy(Gtk::POLICY_AUTOMATIC,
				Gtk::POLICY_AUTOMATIC);
		$this->debug_store = new GtkListStore(GdkPixbuf::gtype,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING);
		$view = new GtkTreeView($this->debug_store);
		$column = new GtkTreeViewColumn('',
				new GtkCellRendererPixbuf(), 'pixbuf', 0);
		$view->append_column($column);
		$column = new GtkTreeViewColumn('Priority',
				new GtkCellRendererText(), 'text', 1);
		$view->append_column($column);
		$column = new GtkTreeViewColumn('Message',
				new GtkCellRendererText(), 'text', 2);
		$view->append_column($column);
		$widget->add($view);
		$this->debug_window->add($widget);
	}

	public function _log_on_delete_event($window)
	{
		$window->hide();
		return TRUE;
	}


	//protected
	//methods
	//rendering
	protected function renderButton($e)
	{
		$stock = $this->_getStock($e->getProperty('stock'));
		if($stock !== FALSE)
			//XXX no longer ignore the original text
			$ret = GtkButton::new_from_stock($stock);
		else
			$ret = new GtkButton($e->getProperty('text'));
		if(($request = $e->getProperty('request')) !== FALSE)
			$ret->connect_simple('clicked', array($this,
						'on_button_clicked'), $request);
		return $ret;
	}

	protected function renderCheckbox($e)
	{
		return new GtkCheckButton($e->getProperty('text'));
	}

	protected function renderDialog($e)
	{
		if(($type = $e->getProperty('type')) === FALSE)
			$type = 'error';
		if(($title = $e->getProperty('title')) === FALSE)
			$title = ucfirst($type);
		$buttons = Gtk::BUTTONS_CLOSE;
		switch($type)
		{
			case 'error':
				$type = Gtk::MESSAGE_ERROR;
				break;
			case 'question':
				$type = Gtk::MESSAGE_QUESTION;
				$buttons = Gtk::BUTTONS_YES_NO;
				break;
			case 'warning':
				$type = Gtk::MESSAGE_WARNING;
				break;
			default:
				$type = Gtk::MESSAGE_INFO;
				$buttons = Gtk::BUTTONS_OK;
				break;
		}
		$dialog = new GtkMessageDialog(NULL, 0, $type, $buttons,
				$title);
		//XXX why is this necessary?
		$dialog->set_title($title);
		if(($text = $e->getProperty('text')) === FALSE)
			$text = '';
		$dialog->set_markup('<b>'.str_replace('<', '&lt;', $title)
				."</b>\n\n".str_replace('<', '&lt;', $text));
		$dialog->connect_simple('delete-event', array($this,
					'on_window_delete_event'), $dialog);
		$dialog->connect_simple('response', array($this,
					'on_window_delete_event'), $dialog);
		$this->windows[] = $dialog;
		$dialog->show();
		return $dialog;
	}

	protected function renderEntry($e)
	{
		$ret = new GtkHbox(FALSE, 4);
		if(($label = $e->getProperty('text')) !== FALSE)
		{
			$label = new GtkLabel($label);
			$label->set_alignment(0.0, 0.5);
			$ret->pack_start($label);
		}
		$entry = new GtkEntry($e->getProperty('value'));
		if($e->getProperty('hidden'))
			$entry->set_visibility(FALSE);
		$ret->pack_start($entry, TRUE, TRUE, 0);
		return $ret;
	}

	protected function renderFileChooser($e)
	{
		$ret = new GtkFileChooser();
		//FIXME really implement
		return $ret;
	}

	protected function renderForm($e)
	{
		//FIXME track the current request for submission
		$ret = new GtkVbox(FALSE, 4);
		$ret->set_border_width(4);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->renderElement($c)) === FALSE)
				continue;
			$ret->pack_start($widget, FALSE, TRUE, 0);
		}
		return $ret;
	}

	protected function renderFrame($e)
	{
		$ret = new GtkFrame($e->getProperty('title'));
		$ret->set_border_width(4);
		$box = $this->renderHbox($e);
		$ret->add($box);
		return $ret;
	}

	protected function renderHbox($e)
	{
		$ret = new GtkHbox(FALSE, 4);
		$ret->set_border_width(4);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->renderElement($c)) === FALSE
					|| $widget instanceof GtkWindow)
				continue;
			$expand = $c->getProperty('Gtk::expand', FALSE);
			$fill = $c->getProperty('Gtk::fill', TRUE);
			if($c->getType() == 'statusbar')
				$ret->pack_end($widget, $expand, $fill, 0);
			else
				$ret->pack_start($widget, $expand, $fill, 0);
			if($c->getType() == 'menubar')
				$ret->reorder_child($widget, 0);
		}
		return $ret;
	}

	protected function renderHtmledit($e)
	{
		//FIXME really implement
		$this->renderTextview($e);
	}

	protected function renderHtmlview($e)
	{
		//FIXME really implement
		$this->renderLabel($e);
	}

	protected function renderIconview($e)
	{
		$e->setProperty('Gtk::expand', TRUE);
		$e->setProperty('Gtk::fill', TRUE);
		$ret = new GtkScrolledWindow();
		$ret->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		$store = new GtkListStore(GdkPixbuf::gtype,
				Gobject::TYPE_STRING);
		$view = new GtkIconView();
		$view->set_model($store);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if($c->getType() != 'row')
				continue;
			$iter = $store->append();
			if(($icon = $c->getProperty('icon')) !== FALSE)
			{
				//FIXME the columns are all strings for now
				if(is_string($icon))
					$store->set($iter, 0, $icon);
			}
			if(($label = $c->getProperty('label')) !== FALSE)
			{
				if($label instanceof GtkButton)
					$label = $label->get_label();
				if($label instanceof GtkLabel)
					$label = $label->get_text();
				if(is_string($label))
					$store->set($iter, 1, $label);
			}
		}
		$ret->add($view);
		return $ret;
	}

	protected function renderImage($e)
	{
		//FIXME really implement
		return new GtkImage;
	}

	protected function renderLabel($e)
	{
		$ret = new GtkLabel($e->getProperty('text'), FALSE);
		$ret->set_alignment(0.0, 0.5);
		return $ret;
	}

	protected function renderLink($e)
	{
		//FIXME create helper function for stock buttons?
		$ret = new GtkButton();
		//FIXME use set_image() and set_label() instead if possible
		$box = new GtkHbox(FALSE, 4);
		$image = GtkImage::new_from_stock(Gtk::STOCK_CONNECT,
				Gtk::ICON_SIZE_BUTTON);
		$box->pack_start($image, FALSE, TRUE, 0);
		$label = new GtkLabel($e->getProperty('text'));
		$label->modify_font($this->fontLink);
		//FIXME setting the color doesn't work
		$label->modify_text(Gtk::STATE_NORMAL, new GdkColor(0, 1.0, 0));
		$box->pack_start($label, TRUE, TRUE, 0);
		$ret->add($box);
		$ret->set_relief(Gtk::RELIEF_NONE);
		$request = $e->getProperty('request');
		$ret->connect_simple('clicked', array($this,
					'on_button_clicked'), $request);
		return $ret;
	}

	protected function renderMenubar($e)
	{
		$ret = new GtkMenuBar;
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if($c->getType() != 'menuitem')
				continue;
			if(($ch = $c->getChildren()) === FALSE
					|| count($ch) == 0)
			{
				//create a sub-menu with a single entry
				$text = $c->getProperty('text');
				$menuitem = new GtkMenuItem($text);
				if(($request = $c->getProperty('request'))
						=== FALSE)
					$menuitem->set_sensitive(FALSE);
				else
				{
					$menu = new GtkMenu;
					$menuitem->set_submenu($menu);
					$m = new GtkMenuItem('This item');
					$m->connect_simple('activate', array(
							$this,
							'on_button_clicked'),
						$request);
					$menu->append($m);
				}
			}
			else
				//create a complete sub-menu
				$menuitem = $this->renderMenuitem($c);
			if($menuitem === FALSE)
				continue;
			$ret->append($menuitem);
		}
		return $ret;
	}

	protected function renderMenuitem($e)
	{
		//FIXME implement images...
		$ret = new GtkMenuItem($e->getProperty('text'));
		$request = $e->getProperty('request');
		if(($children = $e->getChildren()) !== FALSE
				&& count($children))
		{
			$menu = new GtkMenu;
			if($request !== FALSE)
			{
				//XXX find a better label
				$menuitem = new GtkMenuItem('This item');
				$menuitem->connect_simple('activate',
						array($this,
							'on_button_clicked'),
						$request);
				$menu->append($menuitem);
				$menuitem = new GtkSeparatorMenuItem;
				$menu->append($menuitem);
			}
			foreach($children as $c)
			{
				if($c->getType() != 'menuitem')
					continue;
				$menuitem = $this->renderMenuitem($c);
				$menu->append($menuitem);
			}
			$ret->set_submenu($menu);
		}
		else if($request !== FALSE)
			$ret->connect_simple('activate', array($this,
						'on_button_clicked'), $request);
		else
			$ret->set_sensitive(FALSE);
		return $ret;
	}

	protected function renderPage($e)
	{
		$window = new GtkWindow();
		if(($title = $e->getProperty('title')) !== FALSE)
			$window->set_title($title);
		$window->connect_simple('delete-event', array($this,
					'on_window_delete_event'), $window);
		$box = $this->renderVbox($e);
		$box->set_border_width(0);
		$window->add($box);
		$window->show_all();
		$this->windows[] = $window;
		return $window;
	}

	protected function renderStatusbar($e)
	{
		$ret = new GtkStatusBar;
		$ret->push($ret->get_context_id('default'),
				$e->getProperty('text'));
		return $ret;
	}

	protected function renderTextview($e)
	{
		$ret = new GtkTextView;
		//FIXME really implement
		return $ret;
	}

	protected function renderTitle($e)
	{
		$ret = new GtkLabel($e->getProperty('text'), FALSE);
		$ret->set_alignment(0.0, 0.5);
		$ret->modify_font($this->fontTitle);
		return $ret;
	}

	protected function renderToolbar($e)
	{
		//FIXME really implement
		return new GtkToolbar;
	}

	protected function renderTreeview($e)
	{
		$e->setProperty('Gtk::expand', TRUE);
		$e->setProperty('Gtk::fill', TRUE);
		$ret = new GtkScrolledWindow();
		$ret->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title' => _('Title'));
		$keys = array_keys($columns);
		//FIXME patch php-gtk to support dynamically-defined stores
		$store = new GtkListStore(Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING);
		$view = new GtkTreeView($store);
		for($i = 0, $cnt = count($keys); $i < $cnt; $i++)
		{
			$renderer = new GtkCellRendererText();
			$column = new GtkTreeViewColumn($columns[$keys[$i]],
					$renderer, 'text', $i);
			$view->append_column($column);
		}
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if($c->getType() != 'row')
				continue;
			$iter = $store->append();
			for($i = 0, $cnt = count($keys); $i < $cnt; $i++)
			{
				$str = $c->getProperty($keys[$i]);
				if($str instanceof PageElement)
					$str = $str->getProperty('text');
				if(is_bool($str))
					$str = $str ? 'TRUE' : 'FALSE';
				$store->set($iter, $i, $str);
			}
		}
		$ret->add($view);
		return $ret;
	}

	protected function renderVbox($e)
	{
		$ret = new GtkVbox(FALSE, 4);
		$ret->set_border_width(4);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->renderElement($c)) === FALSE
					|| $widget instanceof GtkWindow)
				continue;
			$expand = $c->getProperty('Gtk::expand', FALSE);
			$fill = $c->getProperty('Gtk::fill', TRUE);
			if($c->getType() == 'statusbar')
				$ret->pack_end($widget, $expand, $fill, 0);
			else
				$ret->pack_start($widget, $expand, $fill, 0);
			if($c->getType() == 'menubar')
				$ret->reorder_child($widget, 0);
		}
		return $ret;
	}


	//private
	//methods
	//accessors
	private function _getStock($stock, $fallback = FALSE)
	{
		switch($stock)
		{
			case 'about':
				return Gtk::STOCK_ABOUT;
			case 'add':
				return Gtk::STOCK_ADD;
			case 'admin':
				return Gtk::STOCK_PREFERENCES;
			case 'apply':
				return Gtk::STOCK_APPLY;
			case 'cancel':
				return Gtk::STOCK_CANCEL;
			case 'close':
				return Gtk::STOCK_CLOSE;
			case 'connect':
				return Gtk::STOCK_CONNECT;
			case 'exit':
			case 'logout':
			case 'quit':
				return Gtk::STOCK_QUIT;
			case 'copy':
				return Gtk::STOCK_COPY;
			case 'cut':
				return Gtk::STOCK_CUT;
			case 'paste':
				return Gtk::STOCK_PASTE;
			case 'search':
				return Gtk::STOCK_FIND;
			default:
				if($fallback !== FALSE)
					return $fallback;
				return FALSE;
		}
	}


	//callbacks
	//GtkEngine::on_button_clicked
	public function on_button_clicked($request)
	{
		if(($res = $this->engine->process($request)) !== FALSE)
			$this->renderElement($res);
	}


	//GtkEngine::on_window_delete_event
	public function on_window_delete_event($window)
	{
		if(($res = array_search($window, $this->windows)) !== FALSE)
		{
			unset($this->windows[$res]);
			if(count($this->windows) == 0)
				Gtk::main_quit();
		}
		//FIXME return a boolean?
	}
}

?>
