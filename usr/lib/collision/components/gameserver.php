<?php

require_once('/usr/lib/stampzilla/lib/component.php');

class gameserver extends component {
	public $componentclasses = array('controller');
	public $game = array(
		'mode' => '',
		'duration' => 1,
		'teams' => array(
		),
		'players' => array(
		)
	);
	public $nodes = array();
	private $sent_discover = 0;

	function startup() {

	}

	function event( $pkt ) {
		if ( $pkt['from'] == 'xbee' && isset($pkt['cmd']) ) 
			$this->recived_cmd($pkt);
		else
			print_r($pkt);
	}

	function recived_cmd( $pkt ) {
		switch($pkt['cmd']) {
			case 'node': 
				if ( !isset($this->nodes[$pkt['addr64']]) )
					$this->nodes[$pkt['addr64']] = array();
				
				break;
			case 'nodeType':
				$this->nodes[$pkt['addr64']]['type'] = $pkt['nodeType'];
				$this->nodes[$pkt['addr64']]['online'] = $pkt['online'];
				$this->nodes[$pkt['addr64']]['shots'] = $pkt['shots'];
				$this->nodes[$pkt['addr64']]['life'] = $pkt['life'];

				if ( !$pkt['online'] ) {
					$this->broadcast(array(
						'to' => 'xbee',
						'cmd' => 'welcome',
						'addr' => $pkt['addr64']
					));				
				}
				break;
		}
	}

	function intercom_event($status) {
		if ( $this->sent_discover < time() ) {
			$this->sent_discover = time() + 60;
			$this->broadcast(array(
				'to' => 'xbee',
				'cmd' => 'discover'
			));
		}
	}

	function child() {
		sleep(1);
		$this->intercom(true);
	}	
}

$g = new gameserver();
$g->start('gameserver','child');



?>
