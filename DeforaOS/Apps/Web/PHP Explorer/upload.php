<?php //$Id$
//Copyright (c) 2005-2012 Pierre Pronchery <khorben@defora.org>
//Some parts Copyright (c) 2005 FPconcept (used with permission)
//This file is part of PHP Explorer
//
//PHP Explorer is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, version 3 of the License.
//
//PHP Explorer is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PHP Explorer. If not, see <http://www.gnu.org/licenses/>.



require('./common.php');
require('./config.php');


$folder = filename_safe(stripslashes($_GET['folder']));
if(!$upload)
	$message = 'File uploading is forbidden';
else if(isset($_POST['folder']))
{
	global $root;

	//FIXME foreach does have a syntax to browse an array with its keys
	//FIXME fix stripslashes in newdir.php too
	$newfile = filename_safe(stripslashes($_POST['folder']
			.'/'.$_FILES['file']['name']));
	$tmpfile = filename_safe(stripslashes($_FILES['file']['tmp_name']));
	if($newfile != FALSE
			&& @move_uploaded_file($tmpfile, $root.'/'.$newfile)
			== FALSE)
		$message = 'Could not upload file "'.$newfile.'"';
	else
		$message = 'File "'.$newfile.'" successfully uploaded';
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>Upload file</title>
		<link type="text/css" rel="stylesheet" href="explorer.css"/>
		<script type="text/javascript" src="explorer.js"></script>
	</head>
	<body>
		<h3>File upload</h3>
<?php if(strlen($message)) { ?>
		<p><?php echo html_safe($message); ?></p>
		<!-- FIXME this code won't work without javascript -->
		<!-- FIXME detect if we're in a popup (if not go to explorer.php) -->
		<input type="button" value="Close" onclick="window.close()"/></a>
<?php } else { ?>
		<p>The new file will be uploaded in folder:<br/><i><?php echo html_safe($folder); ?></i></p>
		<form action="upload.php" method="post" enctype="multipart/form-data">
			<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo html_safe($upload_size_max); ?>"/>
			<input type="hidden" name="folder" value="<?php echo html_safe($folder); ?>"/>
			Upload file:
			<input type="file" name="file"/>
			<input type="submit" value="Upload"/>
		</form>
<?php } ?>
	</body>
</html>
