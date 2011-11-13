<?php

require_once('../lib/xbeeApi.php');
require_once('/usr/lib/stampzilla/lib/component.php');

class xbee extends component {
    public $componentclasses = array('gateway');
	private $buff = '';
	private $sh = '';
	private $sl = '';

	function startup() {
		exec("stty -F /dev/ttyUSB0 9600 raw");
		if ( !$this->t = fopen('/dev/ttyUSB0','r+b') )
			die(" - Failed to open\n");

		stream_set_blocking($this->t, 0);
		$this->xapi = new xbeeApi();

		// Request SH and SL
	    fwrite($this->t,$this->xapi->at('SH'));
	    fwrite($this->t,$this->xapi->at('SL'));
	}


	function kill($pkt) {
		note(debug,"Broadcasting exit");
	    fwrite($this->t,$this->xapi->transmit('000000000000ffff',chr(255)));
		$this->ack($pkt);
		$this->kill_child(true);
	}

	function welcome($pkt) {
		$this->responseTo[$this->xapi->id] = $pkt;
	    return fwrite($this->t,$this->xapi->transmit('0013A20040698406',chr(1)));
	}


	function shoot($pkt) {
		$this->responseTo[$this->xapi->id] = $pkt;
	    return fwrite($this->t,$this->xapi->transmit('0013A20040698406',chr(3)));
	}

	function discover($pkt = null) {
		$node = array(
			'cmd' => 'node',
			'addr16' => 'FFFE',
			'addr64' => $this->sh.$this->sl,
			'role' => 0,
			'roleStr' => 'Coordinator',
			'status' => 0,
			'profile' => 0,
			'mf' => ''
		);
		$this->broadcast($node);

		if ( $pkt )
			$this->responseTo[$this->xapi->id] = $pkt;
	    return fwrite($this->t,$this->xapi->at('ND'));
	}

	function at($pkt) {
		if ( !isset($pkt['data']) )
			$pkt['data'] = null;
		
		$this->responseTo[$this->xapi->id] = $pkt;
	    fwrite($this->t,$this->xapi->at($pkt['at'],$pkt['data']));
	}

	function at_event($cmd,$status,$data) {
		switch($cmd) {
			case 'ND':
				$types = array(
					0 => 'Coordinator',
					1 => 'Router',
					2 => 'End device'
				);
				for( $i=0;$i < strlen($data);$i+=20 ) {
					$node = array(
						'cmd' => 'node',
						'addr16' => $this->xapi->decodeAddress(substr($data,$i,2)),
						'addr64' => $this->xapi->decodeAddress(substr($data,$i+2,8)),
						'parent16' => $this->xapi->decodeAddress(substr($data,$i+12,2)),
						'role' => ord(substr($data,$i+14,1)),
						'roleStr' => $types[ord(substr($data,$i+14,1))],
						'status' => ord(substr($data,$i+15,1)),
						'profile' => $this->xapi->decodeHex(substr($data,$i+16,2)),
						'mf' => $this->xapi->decodeHex(substr($data,$i+18,2))
					);
					$this->broadcast($node);
				}
				break;
			case 'SH':
				$this->sh = $this->xapi->decodeAddress($data);
				break;
			case 'SL':
				$this->sl = $this->xapi->decodeAddress($data);
				if ( $this->sh )
					$this->discover();
				break;
			default:
				note(warning,"Unknown AT event ($cmd)");
		}
	}

	function color($pkt) {
		if ( strlen($pkt['color']) == 6 ) {
			$r = hexdec(substr($pkt['color'],0,2));
			$g = hexdec(substr($pkt['color'],2,2));
			$b = hexdec(substr($pkt['color'],4,2));
		} else if ( strlen($pkt['color']) == 3 ) {
			$r = min(hexdec(substr($pkt['color'],0,1))*16,255);
			$g = min(hexdec(substr($pkt['color'],1,1))*16,255);
			$b = min(hexdec(substr($pkt['color'],2,1))*16,255); 
		}

		if ( isset($r) ) {
			note(debug,"Sending color $r,$g,$b");
			$this->responseTo[$this->xapi->id] = $pkt;
		    return fwrite($this->t,$this->xapi->transmit('0013A20040698406',chr(2).chr($r).chr($g).chr($b)));
		}
	}

	function intercom_event($pkt) {
		$pkt = base64_decode($pkt);

		$msg = $this->xapi->decode($pkt);

		if ( !$msg ) 
			return;

		$api = dechex(ord(substr($msg,0,1)));
		$cmdData = substr($msg,1,-1);
		switch( $api ) {
			case '8a':
				$status = ord(substr($cmdData,0,1));
				switch( $status ) {
					case 0:
						$status = 'Hardware reset';
						break;
					case 1:
						$status = 'Watchdog timer reset';
						break;
					case 2:
						$status = 'Joined';
						break;
					case 3:
						$status = 'Unjoined';
						break;
					case 6:
						$status = 'Coordinator started';
						break;
				}
				note(debug,"Recived modem status: $status\n");
				break;
			case '8b':
				$id = ord(substr($cmdData,0,1));
				$from16 = $this->xapi->decodeAddress(substr($cmdData,1,2));
				$retry = ord(substr($cmdData,3,1));
				$delivery = ord(substr($cmdData,4,1));
				$discovery = ord(substr($cmdData,5,1));

				switch( $delivery ) {
					case 0:
						$delivery = "Success";
						break;
					case 2:
						$delivery = "CCA Failure";
						break;
					case 33:
						$delivery = "Network ACK failure";
						break;
					case 34:
						$delivery = "Not joined to network";
						break;
					case 35:
						$delivery = "Self-addressed";
						break;
					case 36:
						$delivery = "Address not found";
						break;
					case 37:
						$delivery = "Route not found";
						break;
				}

				switch( $discovery ) {
					case 0:
						$discovery = "No discovery overhead";
						break;
					case 1:
						$discovery = "Address discovery";
						break;
					case 2:
						$discovery = "Route discovery";
						break;
					case 3:
						$discovery = "Address and route discovery";
						break;
				}

				note(debug,"Recived zigbee status $from16 (retry $retry) -> $delivery -> $discovery");

				if ( isset($this->responseTo[$id]) ) { 
					$this->ack($this->responseTo[$id],substr($cmdData,4,1) == "\x00");
					unset($this->responseTo[$id]);
				}

				break;
			case '88':
				$id = ord(substr($cmdData,0,1));
				$command = substr($cmdData,1,2);
				$status = ord(substr($cmdData,3,1));
				$data = substr($cmdData,4);

				if ( $status ) 
					$msg = "Recived fail response $command ($id): ";
				else {
					$msg = "Recived ok response $command ($id): ";
				}

				for($i=0;$i<strlen($data);$i++) {
					$chr = substr($data,$i,1);
						$msg .= "[".dechex(ord($chr))."]";
				}

				note(debug,$msg);

				$this->at_event($command,$status,$data);
				if ( isset($this->responseTo[$id]) ) { 
					$this->ack($this->responseTo[$id],base64_encode($data));
					unset($this->responseTo[$id]);
				}
				break;
			case '90':
				$from64 = $this->xapi->decodeAddress(substr($cmdData,0,8));
				$from16 = $this->xapi->decodeAddress(substr($cmdData,8,2));
				$options = substr($cmdData,10,1);
				$data = substr($cmdData,11);
				$msg = "Recived packet from $from64 ($from16): $data\n";

				for($i=0;$i<strlen($data);$i++) {
					$msg .= " ".ord(substr($data,$i,1));
				}

				note(debug,$msg);

				break;
			default:
				$msg = "Recived unknown packet:\n";
				$msg .= "  API: $api\n";
				$msg .= "  CMD: ";
				for($i=0;$i<strlen($cmdData);$i++) {
					$chr = substr($cmdData,$i,1);
					if ( ord($chr) > 32 && ord($chr) < 127 ) 
						$msg .= $chr;
					else
						$msg .= "[".dechex(ord($chr))."]";
				}

				note(notice,$msg);
				break;
		}

	}

	function child() {
		$line = fread($this->t,128);
        //for($i=0;$i<strlen($line);$i++)
        //    echo "\033[32m[".dechex(ord(substr($line,$i,1)))."]\033[0m";

        if ( $line === false )
            die(' - Lost connection (child)');
        else {
            if ( strlen($line) > 0 )
                $this->buff .= $line;

            //print_r($pkg);

            if ( ($pos = strpos($this->buff,"~")) !== false ) { // Found start of a package
                //$pos += 1;

                $subpkt = substr($this->buff,$pos);

                $prelen = strlen($subpkt);
				$subpkt = $this->xapi->descape($subpkt);
                $lendiff = strlen($subpkt) - $prelen;

				// Read length
                $msb = ord(substr($subpkt,1,1));
                $lsb = ord(substr($subpkt,2,1));
                $len = $msb*16 + $lsb + 1;

				// Remove header
				$subpkt = substr($subpkt,3);

				//$subpkt = $this->xapi->escape($subpkt);
                if ( $len > 0 ) {
                    $pkt = substr($subpkt,0,$len);
					$chk = dechex(ord(substr($pkt,-1)));

                    /*$msg = "Found pkg, [$msb,$lsb last $chk] $pos, $len";
                    $msg .= " - [$pkt] ".strlen($pkt).'|'.ord(substr($pkt,-1))." ($this->buff) \n";
                    if ( $this->prev != $msg ) {
                        $this->prev = $msg;
                        echo $msg;
                    }*/
                    if ( strlen($pkt) == $len ) {
                        //$pkt = substr($this->buff,$pos-1,3) . $pkt;
                        //echo "GOT IT ($pkt)\n";

                        //$this->xapi->decode($pkt);
						$this->intercom(base64_encode($pkt));
                        $this->buff = substr($this->buff,$pos+$len+4+$lendiff);
                    }
                }
            }
        }

	}
}

$x = new xbee();
$x->start('xbee','child');

?>
