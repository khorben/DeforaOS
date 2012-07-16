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



require_once('./system/mime.php');
require_once('./system/user.php');


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
		$error = _('No CVS repository defined');

		if($this->cvsroot === FALSE || strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		$vbox = new PageElement('vbox');
		//browse
		$path = $this->cvsroot.'/'.$project['cvsroot'];
		if(($file = $request->getParameter('file')) !== FALSE)
		{
			$file = $this->helperSanitizePath($file);
			$path .= "/$file";
		}
		else
			$file = '/';
		$error = _('No such file or directory');
		if(($st = lstat($path)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		if(($st['mode'] & CVSScmProject::$S_IFDIR)
				=== CVSScmProject::$S_IFDIR)
			return $this->_browseDir($engine, $request, $vbox,
					$path, $file);
		if(($revision = $request->getParameter('revision')) !== FALSE)
			return $this->_browseFileRevision($engine, $request,
					$vbox, $path, $file, $revision);
		return $this->_browseFile($engine, $request, $vbox, $path,
				$file);
	}

	private function _browseDir($engine, $request, $vbox, $path, $file)
	{
		$error = _('Could not open directory');

		$vbox->append('title', array('text' => _('Browse source')));
		if(($dir = opendir($path)) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//view
		$columns = array('icon' => '', 'title' => _('Filename'),
				'date' => _('Date'),
				'revision' => _('Revision'),
				'username' => _('Author'),
				'content' => _('Description'));
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
		foreach($folders as $de => $st)
		{
			$row = $view->append('row');
			$icon = new PageElement('image', array('size' => 16,
					'stock' => 'folder'));
			$row->setProperty('icon', $icon);
			//title
			$f = ltrim($file.'/'.$de, '/');
			$r = new Request($engine, $request->getModule(),
					$request->getAction(),
					$request->getId(), $request->getTitle(),
					array('file' => $f));
			$link = new PageElement('link', array('request' => $r,
					'text' => $de));
			$row->setProperty('title', $link);
			//date
			$date = strftime(_('%Y/%m/%d %H:%M:%S'), $st['mtime']);
			$row->setProperty('date', $date);
		}
		foreach($files as $de => $st)
		{
			$row = $view->append('row');
			$icon = Mime::getIcon($engine, $de, 16);
			$icon = new PageElement('image', array(
					'source' => $icon));
			$row->setProperty('icon', $icon);
			//title
			$f = ltrim($file.'/'.$de, '/');
			$r = new Request($engine, $request->getModule(),
					$request->getAction(),
					$request->getId(), $request->getTitle(),
					array('file' => $f));
			$link = new PageElement('link', array('request' => $r,
					'text' => substr($de, 0, -2)));
			$row->setProperty('title', $link);
			//revision
			//FIXME implement
			//date
			$date = strftime(_('%Y/%m/%d %H:%M:%S'), $st['mtime']);
			$row->setProperty('date', $date);
			//author
			//FIXME implement
			//message
			//FIXME implement
		}
		closedir($dir);
		return $vbox;
	}

	private function _browseFile($engine, $request, $vbox, $path, $file)
	{
		$error = _('Could not list revisions');

		//obtain the revisions
		$cmd = 'rlog '.escapeshellarg($path);
		exec($cmd, $rcs, $res);
		if($res != 0 || count($rcs) == 0)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		//view
		$vbox->append('title', array('text' => _('Revisions')));
		$columns = array('title' => _('Revision'), 'date' => _('Date'),
				'username' => _('Author'),
				'message' => _('Message'));
		$view = $vbox->append('treeview', array('columns' => $columns));
		for($i = 0, $cnt = count($rcs); $i < $cnt;)
			if($rcs[$i++] == '----------------------------')
				break;
		for(; $i < $cnt - 2; $i += 3)
		{
			$row = $view->append('row');
			$revision = substr($rcs[$i], 9);
			$r = new Request($engine, 'project', 'browse',
					$request->getId(), $request->getTitle(),
					array('file' => $file,
						'revision' => $revision));
			$link = new PageElement('link', array('request' => $r,
					'text' => $revision));
			$row->setProperty('title', $link);
			$row->setProperty('date', substr($rcs[$i + 1], 6, 19));
			//username
			$username = substr($rcs[$i + 1], 36);
			$username = substr($username, 0, strspn($username,
					'abcdefghijklmnopqrstuvwxyz'
					.'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
					.'0123456789'));
			$user = User::lookup($engine, $username);
			$r = new Request($engine, 'project', 'list',
					$user->getUserId(), $username);
			$link = new PageElement('link', array('request' => $r,
					'stock' => 'user',
					'text' => $username));
			$row->setProperty('username', $link);
			for(; strncmp($rcs[$i + 2], 'branches: ', 10) == 0;
					$i++);
			//message
			$dashes = '----------------------------';
			$longdashes =
'=============================================================================';
			$message = $rcs[$i + 2];
			if($message == $dashes || $message == $longdashes)
				$message = '';
			else
			{
				$msg = '';
				for($i++; $i < $cnt && $rcs[$i + 2] != $dashes
					&& $rcs[$i + 2] != $longdashes; $i++)
					$msg = '...';
				$message .= $msg;
			}
			$row->setProperty('message', $message);
		}
		return $vbox;
	}

	private function _browseFileRevision($engine, $request, $vbox, $path,
			$file, $revision)
	{
		$error = 'Internal server error';

		$cmd = 'co -p'.escapeshellarg($revision).' '.$path;
		if(($fp = popen($cmd, 'r')) === FALSE)
			return new PageElement('dialog', array(
					'type' => 'error', 'text' => $error));
		if($request->getParameter('download') !== FALSE)
			return $fp;
		$label = $vbox->append('label');
		//link back
		$r = new Request($engine, 'project', 'browse',
				$request->getId(), $request->getTitle(),
				array('file' => $file));
		$label->append('link', array('request' => $r, 'stock' => 'back',
					'text' => 'Back to the revision list'));
		//link to the download
		$label->append('label', array('text' => ' '));
		$r = new Request($engine, 'project', 'browse',
				$request->getId(), $request->getTitle(),
				array('file' => $file, 'revision' => $revision,
					'download' => 1));
		$label->append('link', array('request' => $r,
					'stock' => 'download',
					'text' => 'Download file'));
		//link to this page
		$label->append('label', array('text' => ' '));
		$r = new Request($engine, 'project', 'browse',
				$request->getId(), $request->getTitle(), array(
					'file' => $file,
					'revision' => $revision));
		$label->append('link', array('request' => $r,
					'stock' => 'link',
					'text' => 'Permalink'));
		while(($line = fgets($fp)) !== FALSE)
		{
			$line = rtrim($line, "\r\n");
			$vbox->append('label', array(
					'class' => 'preformatted',
					'text' => $line));
		}
		fclose($fp);
		return $vbox;
	}


	//CVSScmProject::download
	public function download($engine, $project, $request)
	{
		$title = _('Repository');
		$repository = 'pserver:'.$this->repository;

		//repository
		if($this->repository === FALSE)
			return FALSE;
		$vbox = new PageElement('vbox');
		$vbox->append('title', array('text' => $title));
		$vbox->append('label', array('text' => _('The source code can be obtained as follows: ')));
		$text = '$ cvs -d:'.$repository.' co '.$project['cvsroot'];
		$vbox->append('label', array('text' => $text,
				'class' => 'preformatted'));
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
		$columns = array('icon' => '', 'title' => _('Filename'),
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


	//protected
	//methods
	//helpers
	//CVSScmProject::helperSanitizePath
	protected function helperSanitizePath($path)
	{
		$path = '/'.ltrim($path, '/');
		$path = str_replace('/./', '/', $path);
		//FIXME really implement '..'
		if(strcmp($path, '/..') == 0 || strpos($path, '/../') !== FALSE)
			return '/';
		return $path;
	}


	//private
	//properties
	static private $S_IFDIR = 040000;
	private $cvsroot = FALSE;
	private $repository = FALSE;
}

?>
