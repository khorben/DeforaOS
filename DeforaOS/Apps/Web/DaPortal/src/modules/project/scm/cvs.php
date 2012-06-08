<?php //$Id$



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
