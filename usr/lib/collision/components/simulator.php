<?php

require_once('/usr/lib/stampzilla/lib/component.php');

function makeAddr64($len = 8) {/*{{{*/
  $addr = '';

  for($i=0;$i<$len;$i++)  
    $addr .= bytehex(chr(rand(0,255)));

  return $addr;
}/*}}}*/
function hexbyte($addr) {/*{{{*/
  $ret = '';
      for($i=0;$i<strlen($addr);$i+=2) {
    $ret .= chr(hexdec(substr($addr,$i,2)));
  }
  return $ret;
}/*}}}*/
function bytehex($addr) {/*{{{*/
  $ret = '';
      for($i=0;$i<strlen($addr);$i+=1) {
    $ret .= str_pad(dechex(ord(substr($addr,$i,1))),2,'0',STR_PAD_LEFT);
  }
  return strtoupper($ret);
}/*}}}*/

class simulator extends component {
  public $componentclasses = array('gateway');
  private $nodes = array();

	function startup() {
    $this->sh = makeAddr64(4);
    $this->sl = makeAddr64(4);

    for($i=0;$i<5;$i++) 
      $this->nodes[] = new galaxy_fighter();
	}

	function event($pkt) {
		if ( $pkt['from'] == 'gameserver' && $pkt['cmd'] == 'bye' ) 
	    	fwrite($this->t,$this->xapi->transmit('000000000000ffff',chr(255)));
	}

	function discover($pkt = null) {/*{{{*/
      $this->broadcast(array(
          'cmd' => 'node',
          'node' => array(
              'cmd' => 'node',
              'addr16' => 'FFFE',
              'addr64' => $this->sh.$this->sl,
              'role' => 0,
              'roleStr' => 'Coordinator',
              'status' => 0,
              'profile' => 0,
              'mf' => ''
          )
      ));

      foreach($this->nodes as $node)
        $this->broadcast($node->broadcast_node());

      foreach($this->nodes as $node)
        $this->broadcast($node->broadcast_nodeType());

      return true;
	}/*}}}*/

	function intercom_event($pkt) {

	}

	function child() {
    
	}
}

class node {/*{{{*/
  public $type = 0;
	function __construct() {/*{{{*/
    $this->addr64 = makeAddr64(8);
    $this->addr16 = makeAddr64(4);
	}/*}}}*/
  function broadcast_node() {/*{{{*/
    $types = array(
      0 => 'Coordinator',
      1 => 'Router',
      2 => 'End device'
    );

    return array(
      'cmd' => 'node',
      'node' => array(
        'addr16' => $this->addr16,
        'addr64' => $this->addr64,
        'parent16' => 'FFFE',
        'role' => 1,
        'roleStr' => $types[1],
        'status' => 0,
        'profile' => 0,
        'mf' => ''
      )
    );
	}/*}}}*/
  function broadcast_nodeType() {/*{{{*/
    return array(
      'cmd' => 'nodeType',
      'addr64' => $this->addr64,
      'nodeType' => $this->type,
      'online' => 1,
      'shots' => 0,
      'life' => 0,
      'color' => 'FEFA21',
    );
	}/*}}}*/
  function broadcast_event($id,$data) {/*{{{*/
    return array(
      'cmd' => 'event',
      'addr64' => $this->addr64,
      'event' => $tmp1[1],
      'data' => $tmp2[1],
    );
  }/*}}}*/
  function broadcast_battery($level) {/*{{{*/
    return array(
      'cmd' => 'battery',
      'addr64' => $this->addr64,
			'level' => ($tmp[1]-456)/0.55,
    );
  }/*}}}*/
}/*}}}*/

class galaxy_fighter extends node {
  public $type = 1;  
}

$x = new simulator();
$x->start('xbee','child');

?>
