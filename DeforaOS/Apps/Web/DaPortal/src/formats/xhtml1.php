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
//TODO:
//- allow the Transitional and Frameset doctypes to be used instead



require_once('./formats/html.php');


//XHTML1Format
class XHTML1Format extends HTMLFormat
{
	//protected
	//methods
	//essential
	//XHTML1Format::match
	protected function match($engine, $type = FALSE)
	{
		switch($type)
		{
			case 'text/html':
				return 80;
			default:
				return 0;
		}
	}


	//XHTML1Format::attach
	protected function attach($engine, $type = FALSE)
	{
		global $config;
		$version = '1.0';
		$encoding = 'UTF-8';
		$dtd = '"-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"';

		parent::attach($engine, $type);
		if(($charset = $config->getVariable('defaults', 'charset'))
				!== FALSE)
			$encoding = $charset;
		$this->doctype = "<?xml version=\"$version\""
			." encoding=\"$encoding\"?>\n";
		$this->doctype .= "<!DOCTYPE html PUBLIC $dtd>\n";
		//for escaping
		if(!defined('ENT_XHTML'))
			define('ENT_XHTML', 0);
	}


	//escaping
	//XHTML1Format::escapeAttribute
	protected function escapeAttribute($text)
	{
		return htmlspecialchars($text, ENT_COMPAT | ENT_XHTML);
	}
}

?>
