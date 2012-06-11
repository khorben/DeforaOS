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


//XHTML11Format
class XHTML11Format extends HTMLFormat
{
	//protected
	//methods
	//essential
	//XHTML11Format::match
	protected function match(&$engine, $type = FALSE)
	{
		switch($type)
		{
			case 'text/html':
				return 90;
			default:
				return 0;
		}
	}


	//XHTML11Format::attach
	protected function attach(&$engine, $type = FALSE)
	{
		global $config;
		$version = '1.0';
		$encoding = 'UTF-8';
		$dtd = '"-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"';

		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			$encoding = $charset;
		$this->doctype = "<?xml version=\"$version\""
			." encoding=\"$encoding\"?>\n";
		$this->doctype .= "<!DOCTYPE html PUBLIC $dtd>\n";
		parent::attach($engine, $type);
	}
}

?>
