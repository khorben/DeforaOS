<?php  //system/mail.php



function _mail($from, $to, $subject, $content, $headers = array())
{
	//FIXME from should be user-defineable
	$hdr = 'From: '.$from.' <'.$_SERVER['SERVER_ADMIN'].">\n";
	if(is_array($headers))
		foreach($headers as $h)
			$hdr .= $h."\n";
	else if(strlen($headers))
		$hdr.=$headers;
	if(!mail($to, $subject, $content, $hdr))
		_error('Could not send mail to: '.$to, 0);
	else
		_info('Mail sent to: '.$to, 0);
}

?>
