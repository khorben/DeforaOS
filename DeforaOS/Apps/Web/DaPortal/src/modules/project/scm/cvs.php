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



//CVSScmProject
class CVSScmProject
{
	//public
	//CVSScmProject::attach
	public function attach(&$engine)
	{
		global $config;

		$this->cvsroot = $config->getVariable('module::project',
			'scm::backend::cvs::cvsroot'); //XXX
		$this->repository = $config->getVariable('module::project',
			'scm::backend::cvs::repository'); //XXX
	}


	//actions
	//CVSScmProject::browse
	public function browse($engine, $project, $request)
	{
		$error = _('Could not open the CVS repository');

		if($this->cvsroot === FALSE || strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('No CVS repository defined')));
		$vbox = new PageElement('vbox');
		//repository
		if($this->repository !== FALSE)
		{
			$title = _('Repository');
			$vbox->append('title', array('text' => $title));
			$vbox->append('label', array('text' => _('The source code can be obtained as follows: ')));
			$text = '$ cvs -d:pserver:'.$this->repository.' co '
				.$project['cvsroot'];
			$vbox->append('label', array('text' => $text,
					'class' => 'preformatted'));
		}
		//browse
		$path = $this->cvsroot.'/'.$project['cvsroot'];
		if(($dir = opendir($path)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		$vbox->append('title', array('text' => _('Browse source')));
		//view
		$columns = array('icon' => '', 'title' => _('Title'),
				'date' => _('Date'));
		$view = $vbox->append('treeview', array('columns' => $columns));
		$folders = array();
		$files = array();
		while(($de = readdir($dir)) !== FALSE)
		{
			if($de == '.' || $de == '..')
				continue;
			if(($st = lstat($path.'/'.$de)) === FALSE)
				continue;
			if(($st['mode'] & CVSScmProject::$S_IFDIR)
					== CVSScmProject::$S_IFDIR)
				$folders[$de] = $st;
			else if(substr($de, -2) != ',v')
				continue;
			else
				$files[$de] = $st;
		}
		ksort($folders);
		ksort($files);
		//XXX code duplication
		foreach($folders as $de => $st)
		{
			$icon = 'folder';
			$row = $view->append('row');
			$icon = new PageElement('image', array('size' => 16,
					'stock' => $icon));
			$row->setProperty('icon', $icon);
			//title
			$r = new Request($engine, $request->getModule(),
					$request->getAction(),
					$request->getId(), $request->getTitle(),
					array('file' => $de));
			$link = new PageElement('link', array('request' => $r,
					'text' => $de));
			$row->setProperty('title', $link);
			//date
			$date = strftime(_('%Y/%m/%d %H:%M:%S'), $st['mtime']);
			$row->setProperty('date', $date);
		}
		foreach($files as $de => $st)
		{
			$icon = 'file';
			$row = $view->append('row');
			$icon = new PageElement('image', array('size' => 16,
					'stock' => $icon));
			$row->setProperty('icon', $icon);
			//title
			$r = new Request($engine, $request->getModule(),
					$request->getAction(),
					$request->getId(), $request->getTitle(),
					array('file' => $de));
			$link = new PageElement('link', array('request' => $r,
					'text' => substr($de, 0, -2)));
			$row->setProperty('title', $link);
			//date
			$date = strftime(_('%Y/%m/%d %H:%M:%S'), $st['mtime']);
			$row->setProperty('date', $date);
		}
		closedir($dir);
		return $vbox;
	}


	//CVSScmProject::timeline
	public function timeline($engine, $project, $request)
	{
		$error = _('No CVS repository defined');

		//check the cvsroot
		if($this->cvsroot === FALSE || strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		if(strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		//history
		$error = _('Could not open the project history');
		$filename = $this->cvsroot.'/CVSROOT/history';
		if(($fp = fopen($filename, 'r')) === FALSE)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		//view
		$columns = array('icon' => '', 'title' => _('Title'),
				'date' => _('Date'), 'action' => _('Action'),
				'revision' => _('Revision'),
				'username' => _('Author'));
		$vbox = new PageElement('vbox');
		$vbox->append('title', array('text' => _('Timeline')));
		$view = $vbox->append('treeview', array(
				'columns' => $columns));
		//rows
		$len = strlen($project['cvsroot']);
		while(($line = fgets($fp)) !== FALSE)
		{
			$fields = explode('|', $line);
			if(strlen($fields[4]) == 0)
				continue;
			if(strncmp($fields[3], $project['cvsroot'], $len) != 0)
				continue;
			$event = FALSE;
			switch($fields[0][0])
			{
				case 'A':
					$event = 'Add';
					$icon = 'add';
					break;
				case 'F':
					$event = 'Release';
					$icon = FALSE;
					break;
				case 'M':
					$event = 'Modify';
					$icon = 'edit';
					break;
				case 'R':
					$event = 'Remove';
					$icon = 'remove';
					break;
			}
			if($event === FALSE)
				continue;
			$row = $view->prepend('row');
			//icon
			$icon = new PageElement('image', array(
					'stock' => $icon, 'size' => 16));
			$row->setProperty('icon', $icon);
			//title
			$title = substr($fields[3], $len ? $len + 1 : $len).'/'
				.$fields[5];
			$title = ltrim($title, '/');
			$title = rtrim($title, "\n");
			$row->setProperty('title', $title);
			//date
			$date = substr($fields[0], 1, 9);
			$date = base_convert($date, 16, 10);
			$date = strftime(_('%d/%m/%Y %H:%M:%S'), $date);
			$row->setProperty('date', $date);
			$row->setProperty('action', $event);
			$row->setProperty('revision', $fields[4]);
			//username
			$username = $fields[1];
			if(($user = User::lookup($engine, $username)) !== FALSE)
			{
				$r = new Request($engine, 'user', FALSE,
						$user->getUserId(),
						$user->getUsername());
				$username = new PageElement('link', array(
						'request' => $r,
						'stock' => 'user',
						'text' => $username));
			}
			$row->setProperty('username', $username);
		}
		//cleanup
		fclose($fp);
		return $vbox;
	}


	//private
	//properties
	static private $S_IFDIR = 040000;
	private $cvsroot = FALSE;
	private $repository = FALSE;
}

?>
