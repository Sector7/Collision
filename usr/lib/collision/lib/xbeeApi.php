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

		if ( strlen($to) == 20 ) {
			$to = $this->parseAddress(substr($to,0,16));
			$local = $this->parseAddress(substr($to,16,4)); // Send to local address
		} else {
			$to = $this->parseAddress($to);
			$local = $this->parseAddress('FFFE'); // Local address is unknown
		}
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

        $len = strlen($cmd);

        $lsb = $len % 256;
        $msb = ($len - $lsb) / 256;

        $pkt = chr($msb).chr($lsb).$cmd.chr($chk);

        $pkt = "\x7E".$this->escape($pkt);

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

	function escape( $pkt ) {
        $pkt = str_replace(
            "\x7D",
            "\x7D\x5D",
            $pkt
        );

        return str_replace(
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
            $pkt
        );
	}

	function descape( $pkt ) {
		$pkt = str_replace(
			"\x7D\x5D",
			"\x7D",
			$pkt
		);

		return str_replace(
			array(
				"\x7D\xDE",
				"\x7D\x31",
				"\x7D\x33",
			),
			array(
				"\xFE",
				"\x11",
				"\x13"
			),
			$pkt
		);
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

	function decodeHex($addr) {
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

		$msg = $in;

        $cmd = substr($msg,0,-1);
        $validate = 0;
        $chk = ord(substr($msg,-1));

        for($i=0;$i<strlen($msg);$i++) {
            $validate += ord($msg[$i]);
        }

		$msg = $this->descape($msg);

        $result = $validate % 256;
        if ( $result != 255 ) {
            echo "Invalid package ($result) $chk: \n";
           for($i=0;$i<strlen($msg);$i++) {
                $chr = substr($msg,$i,1);
                if ( ord($chr) > 32 && ord($chr) < 127 ) 
                    echo $chr;
                else
                    echo "\033[31m[0x".strtoupper(dechex(ord($chr)))."]\033[0m";
            }
            echo "\n";
			return false;
        } else {
			return $msg;
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
