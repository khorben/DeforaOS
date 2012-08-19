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


//Common
class Common
{
	//public
	//properties
	static public $S_IFDIR = 040000;


	//methods
	//static
	//accessors
	//Common::getPermissions
	static public function getPermissions($mode, $ifdir = FALSE)
	{
		if($ifdir === FALSE)
			$ifdir = Common::$S_IFDIR;

		$str = '----------';
		if(($mode & $ifdir) == $ifdir)
			$str[0] = 'd';
		$str[1] = $mode & 0400 ? 'r' : '-';
		$str[2] = $mode & 0200 ? 'w' : '-';
		$str[3] = $mode & 0100 ? 'x' : '-';
		$str[4] = $mode & 040 ? 'r' : '-';
		$str[5] = $mode & 020 ? 'w' : '-';
		$str[6] = $mode & 010 ? 'x' : '-';
		$str[7] = $mode & 04 ? 'r' : '-';
		$str[8] = $mode & 02 ? 'w' : '-';
		$str[9] = $mode & 01 ? 'x' : '-';
		return $str;
	}


	//Common::getSize
	static public function getSize($size)
	{
		if($size < 1024)
			return $size.' '._('bytes');
		if(($size = round($size / 1024)) < 1024)
			return $size.' '._('kB');
		if(($size = round($size / 1024)) < 1024)
			return $size.' '._('MB');
		if(($size = round($size / 1024)) < 1024)
			return $size.' '._('GB');
		return $size.' '._('TB');
	}
}

?>
