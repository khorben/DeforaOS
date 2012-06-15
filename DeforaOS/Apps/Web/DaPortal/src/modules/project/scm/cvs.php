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
		if(strlen($project['cvsroot']) == 0)
			return new PageElement('dialog', array(
				'type' => 'error',
				'text' => _('No CVS repository defined')));
		$view = new PageElement('treeview');
		//FIXME implement
		return $view;
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
