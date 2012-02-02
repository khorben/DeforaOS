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



require_once('./templates/basic.php');


//DeforaOSTemplate
class DeforaOSTemplate extends BasicTemplate
{
	//protected
	//methods
	//accessors
	//DeforaOSTemplate::getMenu
	protected function getMenu($engine, $actions = FALSE)
	{
		if($actions === FALSE)
			$actions = array('news', 'project', 'user' => 'login',
					'wiki');
		return parent::getMenu($engine, $actions);
	}


	//useful
	//DeforaOSTemplate::match
	protected function match(&$engine)
	{
		return 0;
	}


	//DeforaOSTemplate::attach
	protected function attach(&$engine)
	{
		$this->name = 'deforaos';
		parent::attach($engine);
	}
}

?>
