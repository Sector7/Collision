collision = {
	sendColor:function( color ) {
		collision.send("to=xbee&cmd=color&color="+color);
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
				case 'node':
					if ( $('node_'+pkt.addr64) != undefined ) 
						break;

					el = new Element('div', {id: 'node_'+pkt.addr64});
					el.innerHTML = '<h5>'+pkt.addr64+" <span>("+pkt.addr16+")</span></h5>";
	
					if ( $('role'+pkt.role) == undefined ) {
						el2 = new Element('div', {id: 'role'+pkt.role});
						el2.innerHTML = '<h3>'+pkt.roleStr+"</h3><div class=\"nodes\"></div>";
						$('nodes').adopt(el2);

						if ( pkt.role == 0 ) {
							$$('#zbcoord div')[0].style.background = '#0f0';
						}
					}

					$$('#role'+pkt.role+' .nodes')[0].adopt(el);
					break;
				case 'ack':
					switch( pkt.pkt.cmd ) {
					}
					break;
			}
		}
	}
}
