<?php

require_once('xbee.php');

$x = new xbee();
//$x->decode($x->at("NJ"));
//die();

exec("stty -F /dev/ttyUSB0 9600");
if ( !$t = fopen('/dev/ttyUSB0','r+b') )
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

            if ( ($pos = strpos($buff,"~")) !== false ) { // Found start of a package
                $pos += 1;
                $msb = ord(substr($buff,$pos,1));
                $lsb = ord(substr($buff,$pos+1,1));
                $len = $msb*256 + $lsb;

                if ( $len > 0 ) {
                    $subpkt = substr($buff,$pos+2);
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

                    $pkt = substr($subpkt,0,$len+1);

                   /* $msg = "Found pkg, $pos, $len + $lendiff";
                    $msg .= " - [$pkt] ".strlen($pkt)." ($buff) \n";
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


echo "Reading ID:\n";
send($x->at('ID'));
echo "Reading SH:\n";
send($x->at('SH'));
echo "Reading SL:\n";
send($x->at('SL'));

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
        case 'send':
            $ptk = $x->transmit('0013A200407C435C','stamp');
            //$x->decode($ptk);
            fwrite($t,$ptk);
            break;
        default:
            send($x->at(trim($cmd)));
            //send($cmd."\r");
    }
}



fclose($t);
posix_kill($pid,SIGTERM); // Kill child

?>
