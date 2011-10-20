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



//GtkEngine
require_once('./engines/cli.php');
class GtkEngine extends CliEngine
{
	//properties
	private $windows = array();

	private $fontTitle;


	//methods
	//essential
	//GtkEngine::~GtkEngine
	function __destruct()
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
			case 'label':
				return $this->renderLabel($element);
			case 'page':
				return $this->renderWindow($element);
			case 'title':
				return $this->renderTitle($element);
			case 'treeview':
				return $this->renderTreeview($element);
		}
		return FALSE;
	}

	private function renderLabel($e)
	{
		return new GtkLabel($e->getProperty('text'), FALSE);
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

	private function renderWindow($e)
	{
		$window = new GtkWindow();
		$window->set_title($e->getProperty('title'));
		$window->connect_simple('delete-event', array($this,
					'on_window_delete_event'), $window);
		$vbox = new GtkVbox(FALSE, 0);
		$children = $e->getChildren();
		foreach($children as $c)
		{
			if(($widget = $this->render($c)) === FALSE)
				continue;
			$expand = $c->getProperty('Gtk::expand');
			$fill = $c->getProperty('Gtk::fill');
			$vbox->pack_start($widget, $expand, $fill, 4);
		}
		$expand = $e->getProperty('Gtk::expand');
		$fill = $e->getProperty('Gtk::fill');
		if(($text = $e->getProperty('text')) !== FALSE)
			$vbox->pack_start(new GtkLabel($text), $expand, $fill,
					4);
		$window->add($vbox);
		$window->show_all();
		$this->windows[] = $window;
		return $window;
	}


	//protected
	//GtkEngine::match
	protected function match()
	{
		if(getenv('DISPLAY') === FALSE)
			return 0;
		if(class_exists('gtk'))
			return 100;
		return 0;
	}


	//GtkEngine::attach
	protected function attach()
	{
		//initialize fonts
		$this->fontTitle = new PangoFontDescription;
		$this->fontTitle->set_weight('PANGO_WEIGHT_BOLD');
	}


	//GtkEngine::log
	public function log($priority, $message)
	{
		$buttons = array();
		switch($priority)
		{
			case 'LOG_ERR':
				$type = Gtk::MESSAGE_ERROR;
				$title = 'Error';
				if($message == 'Permission denied')
					$buttons[] = array('Authenticate', -1);
				break;
			case 'LOG_DEBUG':
				if(Engine::$debug !== TRUE)
					return;
			case 'LOG_WARNING':
				$type = Gtk::MESSAGE_WARNING;
				$title = 'Warning';
				break;
			default:
				$type = Gtk::MESSAGE_INFO;
				$title = 'Information';
				break;
		}
		$dialog = new GtkMessageDialog(null, 0, $type,
				Gtk::BUTTONS_CLOSE, $message);
		$dialog->set_title($title);
		foreach($buttons as $b)
			$dialog->add_button($b[0], $b[1]);
		$res = $dialog->run();
		$dialog->destroy();
		switch($res)
		{
			case -1:
				$request = new Request($this, 'user', 'login');
				$page = $this->process($request);
				$this->render($page);
				break;
		}
		return FALSE;
	}


	//callbacks
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