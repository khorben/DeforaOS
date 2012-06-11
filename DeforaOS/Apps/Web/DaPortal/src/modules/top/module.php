<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
//This file is part of DaPortal
//
//DaPortal is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License version 2 as
//published by the Free Software Foundation.
//
//DaPortal is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with DaPortal; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



require_once('./system/module.php');


//TopModule
class TopModule extends Module
{
	//public
	//methods
	//TopModule::call
	function call(&$engine, $request)
	{
		if(($action = $request->getAction()) === FALSE)
			$action = 'default';
		switch($action)
		{
			case 'admin':
			case FALSE:
				$action = 'call'.ucfirst($action);
				return $this->$action($engine, $request);
		}
		return FALSE;
	}


	//protected
	//methods
	//calls
	//TopModule::callAdmin
	protected function admin(&$engine, $request)
	{
		$cred = $engine->getCredentials();

		if(!$cred->isAdmin())
			return $engine->log('LOG_ERR', 'Permission denied');
		$title = _('Top links administration');
		//FIXME implement
		return FALSE;
	}


	//TopModule::callDefault
	private function callDefault(&$engine, $request = FALSE)
	{
		//FIXME implement
		return FALSE;
	}
}

?>
