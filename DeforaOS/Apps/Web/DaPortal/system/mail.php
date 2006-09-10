<?php  //system/mail.php



//check url
if(!ereg('/index.php$', $_SERVER['PHP_SELF']))
	exit(header('Location: ../index.php'));


function _mail($from, $to, $subject, $content, $headers = array())
{
	//FIXME from should be user-defineable
	$hdr = 'From: '.$from.' <'.$_SERVER['SERVER_ADMIN'].">\n";
	foreach($headers as $h)
		$hdr .= $h."\n";
	if(!mail($to, $subject, $content, $hdr))
		_error('Could not send mail to: '.$to, 0);
	else
		_info('Mail sent to: '.$to, 0);
}

?>
