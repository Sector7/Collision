<?php

class xbeeApi {
    public $id = 1;

    function at($cmd,$value=null) {
        $cmd = substr($cmd,0,2); // Limit cmd to 2 chars
        $api = "\x08";
        $pkt = $api.chr($this->id).$cmd.$value;

        $this->id++;
        if ( $this->id > 255 )
            $this->id = 1;

        return $this->package($pkt);
    }

    function transmit($to,$data) {
        $api = "\x10";

		$to = $this->parseAddress($to);
		$local = $this->parseAddress('FFFE'); // Local address is unknown
		$radius = "\x00"; // Max radius (10 hops)
		$opts = "\x00"; // 0 = No options, 1 = disable ack, 2 = Disable Network Address Discovery

        $pkt = $api.chr($this->id).$to.$local.$radius.$opts.$data;

        $this->id++;
        if ( $this->id > 255 )
            $this->id = 1;

        return $this->package($pkt);
    }



    function package( $cmd ) {

        $chk = 0;
        for($i=0;$i<strlen($cmd);$i++)
            $chk += ord(substr($cmd,$i,1));
        $chk = 255 - ($chk % 256);

        $cmd = str_replace(
            "\x7D",
            "\x7D\x5D",
            $cmd
        );

        $cmd = str_replace(
            array(
                "\xFE",
                "\x11",
                "\x13"
            ),
            array(
                "\x7D\xDE",
                "\x7D\x31",
                "\x7D\x33",
            ),
            $cmd
        );

        $len = strlen($cmd);

        $lsb = $len % 256;
        $msb = ($len - $lsb) / 256;

        $pkt = "\x7E".chr($msb).chr($lsb).$cmd.chr($chk);

        /*foreach(get_defined_vars() as $key => $line ) {
            echo $key.": ";
            for($i=0;$i<strlen($line);$i++) {
                $chr = substr($line,$i,1);
                if ( ord($chr) > 32 && ord($chr) < 127 ) 
                    echo $chr;
                else
                    echo "[".dechex(ord($chr))."]";
            }
            echo "\n";
        }*/

		return $pkt;
	}

	function parseAddress($addr) {
		$ret = '';
        for($i=0;$i<strlen($addr);$i+=2) {
			$ret .= chr(hexdec(substr($addr,$i,2)));
		}
		return $ret;
	}

	function decodeAddress($addr) {
		$ret = '';
        for($i=0;$i<strlen($addr);$i+=1) {
			$ret .= str_pad(dechex(ord(substr($addr,$i,1))),2,'0',STR_PAD_LEFT);
		}
		return strtoupper($ret);
	}

    function decode( $in ) {

        if ( substr($in,0,1) == "~" ) {
            $in = substr($in,3);
        }

        $msg = str_replace(
            array(
                "\x7D\xDE",
                "\x7D\x5D",
                "\x7D\x31",
                "\x7D\x33",
            ),
            array(
                "\xFE",
                "\x7D",
                "\x11",
                "\x13"
            ),
            $in
        );

        $cmd = substr($msg,0,-1);
        $validate = 0;
        $chk = ord(substr($msg,-1));

        for($i=0;$i<strlen($msg);$i++) {
            $validate += ord($msg[$i]);
        }

        $result = $validate % 256;
        if ( $result != 255 ) {
            echo "Invalid package ($result): \n";
           for($i=0;$i<strlen($msg);$i++) {
                $chr = substr($msg,$i,1);
                if ( ord($chr) > 32 && ord($chr) < 127 ) 
                    echo $chr;
                else
                    echo "\033[31m[0x".strtoupper(dechex(ord($chr)))."]\033[0m";
            }
            echo "\n";
        } else {
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
                        echo "Recived modem status: $status\n";
                        break;
                    case '8b':
                        $id = ord(substr($cmdData,0,1));
                        $from16 = $this->decodeAddress(substr($cmdData,1,2));
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

                        echo "Recived zigbee status $from16 (retry $retry) -> $delivery -> $discovery\n";

                        break;
                    case '88':
                        $id = ord(substr($cmdData,0,1));
                        $command = substr($cmdData,1,2);
                        $status = ord(substr($cmdData,3,1));
                        $data = substr($cmdData,4);
                        if ( $status ) 
                            echo "Recived fail response $command ($id): ";
                        else
                            echo "Recived ok response $command ($id): ";
                        for($i=0;$i<strlen($data);$i++) {
                            $chr = substr($data,$i,1);
                                echo "[".dechex(ord($chr))."]";
                        }
                        echo "\n";
                        break;
                    case '90':
                        $from64 = $this->decodeAddress(substr($cmdData,0,8));
                        $from16 = $this->decodeAddress(substr($cmdData,8,2));
                        $options = substr($cmdData,10,1);
                        $data = substr($cmdData,11);
                        echo "Recived packet from $from64 ($from16): $data\n";
                        break;
                    default:
                        echo "\033[1;34mRecived unknown packet:\n";
                        echo "  API: $api\n";
                        echo "  CMD: ";
                        for($i=0;$i<strlen($cmdData);$i++) {
                            $chr = substr($cmdData,$i,1);
                            if ( ord($chr) > 32 && ord($chr) < 127 ) 
                                echo $chr;
                            else
                                echo "[".dechex(ord($chr))."]";
                        }
                        echo "\033[0m";
                        break;
                }
        }

        /*foreach(get_defined_vars() as $key => $line ) {
            echo " -- ".$key.": ";
            for($i=0;$i<strlen($line);$i++) {
                $chr = substr($line,$i,1);
                if ( ord($chr) > 32 && ord($chr) < 127 and $key != 'msg' ) 
                    echo $chr;
                else
                    echo "[".dechex(ord($chr))."]";
            }
            echo "\n";
        }
        echo "\n";*/
    }
}


?>
