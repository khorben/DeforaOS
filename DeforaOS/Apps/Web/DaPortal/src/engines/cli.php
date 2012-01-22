<?php //$Id$
//Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>
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



require_once('./system/engine.php');


//CliEngine
class CliEngine extends Engine
{
	//public
	//methods
	//accessors
	//CliEngine::getRequest
	public function getRequest()
	{
		if(($options = getopt('Dm:a:i:t:')) === FALSE)
			return FALSE;
		$module = FALSE;
		$action = FALSE;
		$id = FALSE;
		$title = FALSE;
		foreach($options as $key => $value)
			switch($key)
			{
				case 'D':
					$this->setDebug(TRUE);
					break;
				case 'm':
					$module = $options['m'];
					break;
				case 'a':
					$action = $options['a'];
					break;
				case 'i':
					$id = $options['i'];
					break;
				case 't':
					$title = $options['t'];
					break;
			}
		//FIXME also allow parameters to be set
		$ret = new Request($this, $module, $action, $id, $title);
		return $ret;
	}


	//Engine::isIdempotent
	public function isIdempotent()
	{
		return ($_SERVER['REQUEST_METHOD'] == 'POST') ? FALSE : TRUE;
	}


	//essential
	//CliEngine::match
	public function match()
	{
		return 1;
	}


	//CliEngine::attach
	public function attach()
	{
	}


	//useful
	//CliEngine::render
	public function render($page)
	{
		print_r($page);
	}
}

?>
