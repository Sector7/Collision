window.addEvent('load', function() {
	$('iframe').src="incoming.php"
	collision.send('type=hello');
});

collision = {
	nodeTypes: {
		1: 'Standard weapon'
	},
	sendWelcome:function( to ) {
		collision.send("to=xbee&cmd=welcome&addr="+to);
	},
	sendShoot:function( to ) {
		collision.send("to=xbee&cmd=shoot&addr="+to);
	},
	sendColor:function( to ,color ) {
		collision.send("to=xbee&cmd=color&addr="+to+"&color="+color);
	},
	sendDiscover:function() {
		collision.send("to=xbee&cmd=discover");
	},
	send: function( url ) {
		new Request({
			url: "send.php?"+url
		}).send();
	},
	incoming: function(json) {
		pkt = eval('('+json+')');
		if ( pkt.cmd != undefined ) {
			switch(pkt.cmd) {
				case 'greetings':
					if ( pkt.from == 'xbee' ) {
						$$('#zbif div')[0].style.background = '#0f0';
					}
					if ( pkt.from == 'gameserver' ) {
						$$('#ctrl div')[0].style.background = '#0f0';
					}
					break;
				case 'bye':
					if ( pkt.from == 'xbee' ) {
						$$('#zbif div')[0].style.background = '#f00';
						$$('#zbcoord div')[0].style.background = '#f00';
						nodes = $$('.online');
						for ( id in nodes ) {
							if ( nodes[id] == undefined )
								continue

							nodes[id].getElement("div").style.background="#ff0000";
							nodes[id].removeClass('online');
						}
					}
					if ( pkt.from == 'gameserver' ) {
						$$('#ctrl div')[0].style.background = '#f00';
						nodes = $$('.online');
						for ( id in nodes ) {
							if ( nodes[id] == undefined )
								continue

							nodes[id].getElement("div").style.background="#ff0000";
							nodes[id].removeClass('online');
						}
					}
					break;
				case 'nodes':
					opkt = pkt;
					for ( i in opkt.nodes ) {
						if ( opkt.nodes[i].role == undefined )
							continue;

						pkt = opkt.nodes[i];
							if ( pkt.role == 0 ) {
								$$('#zbcoord div')[0].style.background = '#0f0';
							}

							if ( $('node_'+pkt.addr64) != undefined ) 
								break;

							el = new Element('li', {id: 'node_'+pkt.addr64, class: 'new'});
							el.innerHTML = '<div></div>'+pkt.addr64+" <span>("+pkt.addr16+") <span id=\"node_"+pkt.addr64+"_type\"></span></span>";
						
							if ( $('role'+pkt.role) == undefined ) {
								el2 = new Element('div', {id: 'role'+pkt.role});
								el2.innerHTML = '<h3>'+pkt.roleStr+"s</h3><ul class=\"nodes\"></ul>";
								$('nodes').adopt(el2);
							}

							$$('#role'+pkt.role+' .nodes')[0].adopt(el);
							if ( pkt.role != 0 ) {
								el.innerHTML += 
									"<p><input id=\"color_"+pkt.addr64+"\" class=\"color\" onChange=\"collision.sendColor('"+pkt.addr64+"',this.value);\">"+
									"<input type=\"button\" onClick=\"collision.sendWelcome('"+pkt.addr64+"');\" value=\"Welcome\">"+
									"<input type=\"button\" onClick=\"collision.sendShoot('"+pkt.addr64+"');\" value=\"Shoot\"></p>";
								$('color_'+pkt.addr64).color = new jscolor.color($('color_'+pkt.addr64), {});
							}
					}
					break;
				case 'nodeType':
					$("node_"+pkt.addr64+"_type").innerHTML = collision.nodeTypes[pkt.nodeType];
					$("node_"+pkt.addr64).className="online";

					if ( pkt.online ) {
						$$("#node_"+pkt.addr64+" div")[0].style.background="#00ff00";
					} else {
						$$("#node_"+pkt.addr64+" div")[0].style.background="#ff0000";
					}

					$("color_"+pkt.addr64).color.fromString(pkt.color);
					break;
				case 'ack':
					switch( pkt.pkt.cmd ) {
					}
					break;
			}
		}
	}
}
