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



require_once('./formats/html.php');


//HTML5Format
class HTML5Format extends HTMLFormat
{
	//protected
	//methods
	//essential
	//HTML5Format::match
	protected function match(&$engine, $type = FALSE)
	{
		switch($type)
		{
			case 'text/html':
				return 100;
			default:
				return 0;
		}
	}


	//HTML5Format::attach
	protected function attach(&$engine, $type = FALSE)
	{
		$this->doctype = "<!DOCTYPE html>\n";
		parent::attach($engine, $type);
	}
}

?>
