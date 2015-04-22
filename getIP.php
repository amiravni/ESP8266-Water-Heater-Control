<?php
include_once "functions.php";

function printStartEnd($conn,$result) {
	$minStart = 99999;
	$minEnd = 99999;
	if ($result->num_rows > 0) {
		while($row = $result->fetch_assoc()) {
			if (isOutOfdate($row)) {
				EraseQuery($conn,$row["id"]);
				continue;
			}
			$minTmpStart =  round((  strtotime($row["startTime"]) - strtotime(date("Y-m-d H:i:s")) ) / 60);
			$minTMPEnd = round((  strtotime($row["endTime"]) - strtotime(date("Y-m-d H:i:s")) ) / 60);
			if ( $minTmpStart > $minEnd || ($minEnd<99999 && $minTMPEnd < $minEnd) )
			{
				break;
			}
			$minEnd = $minTMPEnd;
			if ($minStart==99999) {$minStart =  $minTmpStart;}
		} 
	}

	echo 'NextStart:'.$minStart. "\r\n".'Next_End_:' .$minEnd;
}

if ( sha1($_GET['psswd']) == PWD_SHA)	
{
	$ip = $_SERVER['REMOTE_ADDR'];
	$filename = "myIPadd.txt";
	file_put_contents($filename, $ip, LOCK_EX);
	$myState = strtotime(date("Y-m-d H:i:s")). " ".$_GET['myState'];
	$filename = "myState.txt";
	file_put_contents($filename, $myState, LOCK_EX);
	//echo 'DDD <br>';
	$conn = startConnection();
	printStartEnd($conn,getTable($conn));
}	


?>