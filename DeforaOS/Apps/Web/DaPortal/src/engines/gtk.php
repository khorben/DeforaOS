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


//GtkEngine
class GtkEngine extends CliEngine
{
	//private
	//properties
	private $windows = array();
	private $debug_window = FALSE;
	private $debug_store;

	private $fontTitle;
	private $fontLink;


	//methods
	//essential
	//GtkEngine::~GtkEngine
	public function __destruct()
	{
		if(count($this->windows) == 0)
			return;
		//enter the main loop
		Gtk::main();
		foreach($this->windows as $w)
			$w->destroy();
	}


	//public
	//useful
	//GtkEngine::render
	public function render($element)
	{
		$element->setProperty('Gtk::expand', FALSE);
		$element->setProperty('Gtk::fill', TRUE);
		switch($element->getType())
		{
			case 'button':
				return $this->renderButton($element);
			case 'checkbox':
				return $this->renderCheckbox($element);
			case 'dialog':
				return $this->renderDialog($element);
			case 'entry':
				return $this->renderEntry($element);
			case 'form':
				return $this->renderForm($element);
			case 'frame':
				return $this->renderFrame($element);
			case 'hbox':
				return $this->renderHbox($element);
			case 'iconview':
				return $this->renderIconview($element);
			case 'label':
				return $this->renderLabel($element);
			case 'link':
				return $this->renderLink($element);
			case 'menubar':
				return $this->renderMenubar($element);
			case 'menuitem':
				return $this->renderMenuitem($element);
			case 'page':
				return $this->renderWindow($element);
			case 'statusbar':
				return $this->renderStatusbar($element);
			case 'title':
				return $this->renderTitle($element);
			case 'treeview':
				return $this->renderTreeview($element);
			case 'vbox':
				return $this->renderVbox($element);
		}
		return FALSE;
	}

	private function renderButton($e)
	{
		return new GtkButton($e->getProperty('text'));
	}

	private function renderCheckbox($e)
	{
		return new GtkCheckButton($e->getProperty('text'));
	}

	private function renderDialog($e)
	{
		switch(($title = $e->getProperty('title')))
		{
			case 'Error':
				$type = Gtk::MESSAGE_ERROR;
				break;
			case 'Question':
				$type = Gtk::MESSAGE_QUESTION;
				break;
			case 'Warning':
				$type = Gtk::MESSAGE_WARNING;
				break;
			case FALSE:
				$title = 'Dialog';
			default:
				$type = Gtk::MESSAGE_INFO;
				break;
		}
		$dialog = new GtkMessageDialog(NULL, 0, $type,
				Gtk::BUTTONS_OK, $title);
		if(($text = $e->getProperty('text')) === FALSE)
			$text = '';
		$dialog->set_markup('<b>'.str_replace('<', '&lt;', $title)
				."</b>\n\n".str_replace('<', '&lt;', $text));
		$dialog->connect_simple('delete-event', array($this,
					'on_window_delete_event'), $dialog);
		$this->windows[] = $dialog;
		$dialog->show();
		return $dialog;
	}

	private function renderEntry($e)
	{
		$ret = new GtkHbox(FALSE, 4);
		if(($label = $e->getProperty('text')) !== FALSE)
			$ret->pack_start(new GtkLabel($label), FALSE, TRUE, 0);
		$entry = new GtkEntry($e->getProperty('value'));
		if($e->getProperty('hidden'))
			$entry->set_visibility(FALSE);
		$ret->pack_start($entry, TRUE, TRUE, 0);
		return $ret;
	}

	private function renderForm($e)
	{
		//FIXME track the current request for submission
		$ret = new GtkVbox(FALSE, 0);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->render($c)) === FALSE)
				continue;
			$ret->pack_start($widget, FALSE, TRUE, 0);
		}
		return $ret;
	}

	private function renderFrame($e)
	{
		$ret = new GtkFrame($e->getProperty('title'));
		$ret->set_border_width(4);
		$box = $this->renderHbox($e);
		$ret->add($box);
		return $ret;
	}

	private function renderHbox($e)
	{
		$ret = new GtkHbox(FALSE, 0);
		$ret->set_border_width(4);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->render($c)) === FALSE)
				continue;
			$expand = $c->getProperty('Gtk::expand', FALSE);
			$fill = $c->getProperty('Gtk::fill', TRUE);
			if($c->getType() == 'statusbar')
				$ret->pack_end($widget, $expand, $fill, 4);
			else
				$ret->pack_start($widget, $expand, $fill, 4);
		}
		return $ret;
	}

	private function renderIconview($e)
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
				$store->set($iter, 0, $c->getProperty('icon'));
			$store->set($iter, 1, $c->getProperty('label'));
		}
		$ret->add($view);
		return $ret;
	}

	private function renderLabel($e)
	{
		return new GtkLabel($e->getProperty('text'), FALSE);
	}

	private function renderLink($e)
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

	private function renderMenubar($e)
	{
		$ret = new GtkMenuBar;
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if($c->getType() != 'menuitem')
				continue;
			if(($menuitem = $this->render($c)) === FALSE)
				continue;
			$ret->append($menuitem);
		}
		return $ret;
	}

	private function renderMenuitem($e)
	{
		$ret = new GtkMenuItem($e->getProperty('text'));
		//FIXME implement images...
		return $ret;
	}

	private function renderStatusbar($e)
	{
		$ret = new GtkStatusBar;
		$ret->push($ret->get_context_id('default'),
				$e->getProperty('text'));
		return $ret;
	}

	private function renderTitle($e)
	{
		$ret = new GtkLabel($e->getProperty('text'), FALSE);
		$ret->set_alignment(0.0, 0.5);
		$ret->modify_font($this->fontTitle);
		return $ret;
	}

	private function renderTreeview($e)
	{
		$e->setProperty('Gtk::expand', TRUE);
		$e->setProperty('Gtk::fill', TRUE);
		$ret = new GtkScrolledWindow();
		$ret->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		$columns = $e->getProperty('columns');
		if(!is_array($columns) || count($columns) == 0)
			$columns = array('title');
		//FIXME patch php-gtk to support dynamically-defined stores
		$store = new GtkListStore(Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING, Gobject::TYPE_STRING,
				Gobject::TYPE_STRING);
		$view = new GtkTreeView($store);
		for($i = 0, $cnt = count($columns); $i < $cnt; $i++)
		{
			$renderer = new GtkCellRendererText();
			$column = new GtkTreeViewColumn(ucfirst($columns[$i]),
					$renderer, 'text', $i);
			$view->append_column($column);
		}
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if($c->getType() != 'row')
				continue;
			$iter = $store->append();
			for($i = 0, $cnt = count($columns); $i < $cnt; $i++)
				$values[] = $store->set($iter, $i,
						$c->getProperty($columns[$i]));
		}
		$ret->add($view);
		return $ret;
	}

	private function renderVbox($e)
	{
		$ret = new GtkVbox(FALSE, 0);
		$ret->set_border_width(4);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->render($c)) === FALSE)
				continue;
			$expand = $c->getProperty('Gtk::expand', FALSE);
			$fill = $c->getProperty('Gtk::fill', TRUE);
			if($c->getType() == 'statusbar')
				$ret->pack_end($widget, $expand, $fill, 4);
			else
				$ret->pack_start($widget, $expand, $fill, 4);
		}
		return $ret;
	}

	private function renderWindow($e)
	{
		$window = new GtkWindow();
		$window->set_title($e->getProperty('title'));
		$window->connect_simple('delete-event', array($this,
					'on_window_delete_event'), $window);
		$box = $this->renderVbox($e);
		$box->set_border_width(0);
		$window->add($box);
		$window->show_all();
		$this->windows[] = $window;
		return $window;
	}


	//GtkEngine::match
	public function match()
	{
		if(getenv('DISPLAY') === FALSE)
			return 0;
		if(class_exists('gtk'))
			return 100;
		return 0;
	}


	//GtkEngine::attach
	public function attach()
	{
		//initialize fonts
		$this->fontTitle = new PangoFontDescription;
		$this->fontTitle->set_weight('PANGO_WEIGHT_BOLD');
		$this->fontLink = new PangoFontDescription;
	}


	//GtkEngine::log
	public function log($priority, $message)
	{
		$icon = Gtk::STOCK_DIALOG_INFO;
		$title = 'Information';
		switch($priority)
		{
			case 'LOG_DEBUG':
				if(Engine::$debug !== TRUE)
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
		if(Engine::$debug === TRUE)
			$this->_log_append($icon, $priority, $message);
		if($priority == 'LOG_DEBUG')
			return FALSE;
		return new PageElement('dialog', array('title' => $title,
					'text' => $message));
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


	//callbacks
	//GtkEngine::on_button_clicked
	public function on_button_clicked($request)
	{
		if(($res = $request->process($this)) !== FALSE)
			$this->render($res);
	}


	//GtkEngine::on_window_delete_event
	public function on_window_delete_event($window)
	{
		if(($res = array_search($window, $this->windows)) !== FALSE)
		{
			unset($this->windows[$res]);
			if(count($this->windows == 0))
				Gtk::main_quit();
		}
	}
}

?>
