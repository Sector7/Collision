<?php

require_once('../lib/xbeeApi.php');
require_once('/usr/lib/stampzilla/lib/component.php');

$xapi = new xbeeApi();

class xbee extends component {
    public $componentclasses = array('gateway');
}

//$x = new xbee();
//$x->start('xbee');
$x = new xbeeApi();







//$x->decode($x->at("NJ"));
//die();

exec("stty -F /dev/ttyUSB1 9600");
if ( !$t = fopen('/dev/ttyUSB1','r+b') )
    die(" - Failed to open\n");

stream_set_blocking($t, 0);

$pid = pcntl_fork();

if ( $pid == -1 )
    die(" - Failed to fork");

if ( !$pid ) { // Child
    $buff = '';
    while(true) {
        $line = fread($t,128);
        //for($i=0;$i<strlen($line);$i++)
        //    echo "\033[32m[".dechex(ord(substr($line,$i,1)))."]\033[0m";

        if ( $line === false )
            die(' - Lost connection (child)');
        else {
            if ( strlen($line) > 0 )
                $buff .= $line;

            //print_r($pkg);

            if ( ($pos = strpos($buff,"~")) !== false ) { // Found start of a package
                $pos += 1;

                $subpkt = substr($buff,$pos);
                $prelen = strlen($subpkt);
                $subpkt = str_replace(
                    "\x7D\x5D",
                    "\x7D",
                    $subpkt
                );

                $subpkt = str_replace(
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
                    $subpkt
                );

                $lendiff = strlen($subpkt) - $prelen;

                $msb = ord(substr($subpkt,0,1));
                $lsb = ord(substr($subpkt,1,1));
                $len = $msb*16 + $lsb;

				$subpkt = substr($subpkt,2);
				$subpkt = $x->escape($subpkt);
                if ( $len > 0 ) {
                    $pkt = substr($subpkt,0,$len+1);

                    /*$msg = "Found pkg, [".ord(substr($buff,$pos-1,1)).",$msb,$lsb] $pos, $len";
                    $msg .= " - [$pkt] ".strlen($pkt).'|'.ord(substr($pkt,-1))." ($buff) \n";
                    if ( $prev != $msg ) {
                        $prev = $msg;
                        echo $msg;
                    }*/
                    if ( strlen($pkt)-1 == $len ) {
                        //$pkt = substr($buff,$pos-1,3) . $pkt;
                        //echo "GOT IT ($pkt)\n";

                        $x->decode($pkt);
                        $buff = substr($buff,$pos+$len+5+$lendiff);
                    }
                }
            }
        }
    }
    die();
}


// -------------------- PARENT ------------------------
function send($txt) {
    global $t;
    $txt2 = trim($txt);
    //echo "UT: $txt2\n";

    fwrite($t,rtrim($txt,"\n")."\r");
    usleep(1000000);
    
    /*for($i=0;$i<strlen($txt);$i++) {
        fwrite($t,$txt);
    }*/
}


/*echo "Reading ID:\n";
send($x->at('ID'));
echo "Reading SH:\n";
send($x->at('SH'));
echo "Reading SL:\n";
send($x->at('SL'));
*/

$stdin = fopen('php://stdin', 'r');
while(true) {
    echo "\n";
    $cmd = trim(fread($stdin,1000));
    switch($cmd) {
        case 'bye':
            break 2;
        case 'start':
            fwrite($t,"+");
            fwrite($t,"+");
            fwrite($t,"+");
            sleep(1);
            break;
        case 'NJ':
            send($x->at('NJ'));
            break;
        case 'send0':
            //$ptk = $x->transmit('0013A200407C435C','stamp');
            //$ptk =   $x->transmit('0013A20040698406',"\x10");
            $ptk =   $x->transmit('000000000000FFFF',"\x00");
            //$x->decode($ptk);
            fwrite($t,$ptk);
            break;
        case 'send1':
            //$ptk = $x->transmit('0013A200407C435C','stamp');
            $ptk =   $x->transmit('0013A20040698406',"\x10");
            //$ptk =   $x->transmit('000000000000FFFF',"\xFF");
            //$x->decode($ptk);
            fwrite($t,$ptk);
            break;
		case 'fade':
			while(1) {
				for($i=0;$i<256;$i+=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr($i).chr(0).chr(0)));
					usleep(25000);
				}
				for($i=255;$i>0;$i-=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr($i).chr(0).chr(0)));
					usleep(25000);
				}
				for($i=0;$i<256;$i+=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr(0).chr($i).chr(0)));
					usleep(25000);
				}
				for($i=255;$i>0;$i-=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr(0).chr($i).chr(0)));
					usleep(25000);
				}
				for($i=0;$i<256;$i+=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr(0).chr(0).chr($i)));
					usleep(25000);
				}
				for($i=255;$i>0;$i-=5) {
					echo "Send $i\n";
	    	        fwrite($t,$x->transmit('0013A20040698406',chr(0).chr(0).chr($i)));
					usleep(25000);
				}
			}
            break;
        case 'setup':
            send($x->at('AP',"\x02"));
            break;
        default:
            send($x->at(trim($cmd)));
            //send($cmd."\r");
    }
}



fclose($t);
posix_kill($pid,SIGTERM); // Kill child

?>
