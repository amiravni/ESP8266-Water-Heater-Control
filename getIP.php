<?php
include_once "functions.php";

function checkStateChage($stateOnly) {
	$stateFile = fopen(myStateName, "r") or die("Unable to open file!");
	$stringFromFile = fread($stateFile,filesize(myStateName));
	fclose($stateFile);
	$pos = strpos($stringFromFile, " ");
//	$lastTime =  date("Y-m-d H:i:s",substr($stringFromFile, 0, $pos));
	$lastState = substr($stringFromFile, $pos+1,1);
	if ($stateOnly != $lastState) {
		if ($stateOnly == 1) {
			$ch = curl_init("http://api.pushingbox.com/pushingbox?devid=".Notification_ON);
			curl_exec ($ch);
			curl_close ($ch);
		}
		elseif ($stateOnly == 0) {
			$ch = curl_init("http://api.pushingbox.com/pushingbox?devid=".Notification_OFF);
			curl_exec ($ch);
			curl_close ($ch);
		}
	}	
}

function printStartEnd($conn,$result) {
	$minStart = 99999;
	$minEnd = 99999;
	if ($result->num_rows > 0) {
		while($row = $result->fetch_assoc()) {
			if (handleDailyQuery($conn,$row) === FALSE) {
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
	$filename = IPtextName;
	file_put_contents($filename, $ip, LOCK_EX);
	$stateOnly = $_GET['myState'];
	$lastReset = $_GET['lastReset'];
	if ($lastReset>0) {
		checkStateChage($stateOnly);
	}
	$myState = strtotime(date("Y-m-d H:i:s")). " ".$stateOnly." ".$lastReset;
	$filename = myStateName;
	file_put_contents($filename, $myState, LOCK_EX);
	$conn = startConnection();
	printStartEnd($conn,getTable($conn));
}	


?>