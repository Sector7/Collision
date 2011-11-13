<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">  
    <head>
        <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
        <meta http-equiv="X-UA-Compatible" content="chrome=1">
        <meta http-equiv="Expires" content="Tue, 01 Jan 1980 1:00:00 GMT">
        <meta http-equiv="Pragma" content="no-cache"> 
        <title>Collision - Lasergame made awsome!</title>

        <script type="text/javascript" src="js/mootools-core-1.3-full-compat-yc.js"></script>
        <script type="text/javascript" src="js/jscolor/jscolor.js"></script>
        <script type="text/javascript" src="js/collision.js"></script>

        <link href="css/base.css" rel="stylesheet" />
    </head>
    <body>
        <div id="right">
            <div class="status box">
                <h2>System status</h2>
                <ul>
                    <li id="zbif"><div></div>ZigBee interface</li>
                    <li id="zbcoord" onClick="collision.send('to=xbee&cmd=kill')"><div></div>ZigBee coordinator</li>
                    <li id="ctrl" onClick="collision.send('to=gameserver&cmd=kill')"><div></div>Game controller</li>
                </ul>
            </div>
            <div class="network box">
                <h2>Network
                <input type="button" value="Discover" onClick="collision.sendDiscover();">
                </h2>
				<div id="nodes"></div>
            </div>
        </div>
        <div id="left">
            <div class="newgame box">
                <h2>Create a new game</h2>
                <form>
					<label>Game mode</label>
					<select name="mode">
						<option value="teamscore">Team - Score</option>
						<option value="teamdm">Team - Deathmatch</option>
						<option value="capture">Team - Capture the flag</option>
						<option value="score">Score</option>
						<option value="dm">Deathmatch</option>
					</select>

					<label>Game duration (minutes)</label>
					<input type="text" name="duration">

					<label>Number of teams</label>
					<input type="text" name="teams">


					<input type="submit" value="Create game">
				</form>
            </div>
			<div class="game box">
				<h2>Active game (Team - Score)</h2>
			</div>
            <div class="players box">
                <h2>Players</h2>
				<table cellspacing=0> 
					<tr>
						<th class="name">Name</th>
						<th class="vest">Vest</th>
						<th class="weapon">Weapon</th>
						<th>Team</th>
						<th>Kills</th>
						<th>Deaths</th>
						<th>Score</th>
					</tr>

					<tr>
						<td><input type="text"></td>
						<td>0013A20040698406</td>
						<td>0013A20040698406</td>
						<td>Team 1</td>
						<td>0</td>
						<td>0</td>
						<td>0</td>
					</tr>
					<tr class="even">
						<td><input type="text"></td>
						<td>0013A20040698406</td>
						<td>0013A20040698406</td>
						<td>Team 1</td>
						<td>0</td>
						<td>0</td>
						<td>0</td>
					</tr>

					<tr>
						<td><input type="text"></td>
						<td>0013A20040698406</td>
						<td>0013A20040698406</td>
						<td>Team 1</td>
						<td>0</td>
						<td>0</td>
						<td>0</td>
					</tr>
					<tr class="even">
						<td><input type="text"></td>
						<td>0013A20040698406</td>
						<td>0013A20040698406</td>
						<td>Team 1</td>
						<td>0</td>
						<td>0</td>
						<td>0</td>
					</tr>
				</table>
            </div>
            <div class="box" style="display:none;">
                <h2>Debug</h2>
				<p>
					<img src="http://www.asciitable.com/index/asciifull.gif">
				</p>
            </div>
        </div>
		<iframe id="iframe" style="display:none;"></iframe>
    </body>
</html>

