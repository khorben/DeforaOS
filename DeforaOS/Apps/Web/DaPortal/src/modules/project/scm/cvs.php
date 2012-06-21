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
		if(strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('No CVS repository defined')));
		$columns = array('title' => _('Title'), 'date' => _('Date'),
				'action' => _('Action'),
				'revision' => _('Revision'),
				'username' => _('Author'));
		$view = new PageElement('treeview', array(
				'columns' => $columns));
		//FIXME implement
		return $view;
	}
}

?>
