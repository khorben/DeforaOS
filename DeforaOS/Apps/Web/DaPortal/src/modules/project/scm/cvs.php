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
	}


	//actions
	//CVSScmProject::browse
	public function browse($engine, $project, $request)
	{
		global $config;
		$cvsroot = $config->getVariable('module::project',
			'scm::backend::cvs::cvsroot'); //XXX
		$repository = $config->getVariable('module::project',
			'scm::backend::cvs::repository'); //XXX

		if($cvsroot === FALSE || strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('No CVS repository defined')));
		$vbox = new PageElement('vbox');
		if($repository !== FALSE)
		{
			$vbox->append('title', array(
					'text' => _('Repository')));
			$vbox->append('label', array('text' => _('The source code can be obtained as follows: ')));
			$text = '$ cvs -d:pserver:'.$repository.' co '.$project['cvsroot'];
			$vbox->append('label', array('text' => $text));
		}
		$vbox->append('title', array('text' => _('Browse source')));
		$view = $vbox->append('treeview');
		//FIXME implement
		return $vbox;
	}


	//CVSScmProject::timeline
	public function timeline($engine, $project, $request)
	{
		global $config;
		$cvsroot = $config->getVariable('module::project',
			'scm::backend::cvs::cvsroot'); //XXX
		$error = _('No CVS repository defined');

		//check the cvsroot
		if($cvsroot === FALSE || strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		if(strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error', 'text' => $error));
		//history
		$error = _('Could not open the project history');
		if(($fp = fopen($cvsroot.'/CVSROOT/history', 'r')) === FALSE)
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
}

?>
