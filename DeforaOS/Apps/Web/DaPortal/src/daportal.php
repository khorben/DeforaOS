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



require_once('./system/config.php');
global $config;
$config = new Config();
if(!isset($daportalconf))
	$daportalconf = '../daportal.conf';
$config->load($daportalconf);

require_once('./system/engine.php');
if(($engine = Engine::attachDefault()) !== FALSE
		&& ($request = $engine->getRequest()) !== FALSE)
{
	$page = $engine->process($request);
	$engine->render($page);
}
unset($engine);

?>
