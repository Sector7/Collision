<?php

require "/usr/lib/stampzilla/lib/constants.php";
require "/usr/lib/stampzilla/lib/udp.php";
require_once("/usr/lib/stampzilla/lib/spyc.php");

echo str_pad('',4096,' ');
ob_flush();
flush();

$logger = new logger();

class logger {
	function __construct() {
        $this->network = spyc_load_file('/etc/stampzilla/network.yml');

        if(!isset($this->network['listen']) )
            $this->network['listen'] = '0.0.0.0';
        if(!isset($this->network['broadcast']) )
            $this->network['broadcast'] = '255.255.255.255';
        if(!isset($this->network['port']) )
            $this->network['port'] = '8282';

        print_r($this);

		// Create a new udp socket
        $this->udp = new udp($this->network['listen'],$this->network['broadcast'],$this->network['port']);

		while(1) {
			$this->parent++;
			if ( !$pkt = $this->udp->listen() )
				continue;

            $p = json_encode($pkt);
			// Format message
			$msg = "\n".$p.'<br /><script language="javascript">parent.collision.incoming("'.addslashes($p).'");</script>';

			echo str_pad($msg,4096,' ',STR_PAD_LEFT);
			ob_flush();
			flush();
		}
	}
}



?>
