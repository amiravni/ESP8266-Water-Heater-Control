<?php
include_once "functions.php";

function addHeader() {
	echo '<html><head>';
	echo '<title>Amir and Batia Water Heater Schedule</title>';
	echo '<meta charset=utf-8 />   <meta name="viewport" content="width=device-width, initial-scale=1">';
	echo ' <style id="jsbin-css">
    .tableRow {
		margin-left: auto;
		margin-right: auto; 
    }
	.tableCell {
		margin-left: auto;
		margin-right: auto; 
		text-align: center;		
		border: 2px solid black;
	}
	</style>';
	echo '</head>';
	echo '<body><div align="center"><h2>Amir and Batia Water Heater Schedule</h2></div>';
	
}

function addFooter() {
	echo '</body></html>';
}


function drawTableCell($val) {
	echo '<td class="tableCell">' . $val . '</td>';
}

function drawTableRow($row) {
	echo '<tr class="tableRow">';
	$formID = '"toErase'.$row["id"].'"';
	drawTableCell('<form id='.$formID.' method="POST">
	<input type="hidden" name="Command" value="Erase">
	<input type="hidden" name="id" value="'. $row["id"] .'">'.	
	"<button onclick='submit_Form(".$formID.")'>Erase</button>".
	'</form>');
	drawTableCell($row["id"]);
	drawTableCell($row["name"]);
	drawTableCell($row["toRepeat"]);
	drawTableCell($row["startTime"]);
	drawTableCell($row["endTime"]);
	echo '</tr>';
}

function printTable($conn,$result) {


	echo '<div align="center"> Current Schedule: <br> <table id="mainTable"><tr>';
	drawTableCell("");
	drawTableCell("id");
	drawTableCell("name");
	drawTableCell("toRepeat");
	drawTableCell("startTime");
	drawTableCell("endTime");
	echo '</tr>';
	
	if ($result->num_rows > 0) {
		// output data of each row
		while($row = $result->fetch_assoc()) {
			if (isOutOfdate($row)) {
				EraseQuery($conn,$row["id"]);
			}
			else {
				drawTableRow($row);
			}
		}
	} else {
		echo "0 results";
	}
	echo '</table></div>';
}

function addQuerySelect($conn,$sVar) {
	$stmt = $conn->prepare("INSERT INTO ".NAME_DB_TOUSE." (name, toRepeat, startTime, endTime) VALUES (?, ?, ?, ?)");
	$stmt->bind_param("ssss", $sVar->name, $sVar->toRepeat, $sVar->startTime,$sVar->endTime);
	$result = $stmt->execute();
	$stmt->close();
	return $result;
}

function getVariablesAdd($sVar){
	$sVar->name = ($_POST['myName']);
	$sVar->toRepeat = ($_POST['myRepeat']);
	$sVar->startTime = date(($_POST['startDate'] .' '. $_POST['startTime'] ));
//	$sVar->endTime = $sVar->startTime;
	$sVar->endTime =date('Y-m-d H:i', strtotime('+'. $_POST['Duration'] .' minutes',strtotime($sVar->startTime)));
	return $sVar;
}

function getVariablesErase($sVar){
	$sVar->id = ($_POST['id']);
	return $sVar;
}

function getVariablesUpdate($sVar){
	$sVar->action = ($_POST['myAction']);
	return $sVar;
}


function checkVariables(){
	
	if (  strlen($_POST['myName']) < 6  &&
		 strlen($_POST['myRepeat']) < 6 &&
		 (strlen($_POST['startDate']) == 10 || strlen($_POST['startDate']) == 0) &&
		 (strlen($_POST['startTime']) == 5 || strlen($_POST['startTime']) == 0) &&
		 strlen($_POST['Duration']) < 4 &&
		 strlen($_POST['myAction']) < 30 &&
		 strlen($_POST['Command']) < 7 &&
		 strlen($_POST['id']) < 6 )
		 {
		return 34566543;
	}
	return 1;
}

$sVar = array(
	'Command' => NULL,
	'id' => NULL,
	'name' => NULL,
	'toRepeat' => NULL,
	'startTime' => NULL,
	'endTime' => NULL,
	'action' => NULL
);

addHeader();
$conn = startConnection();

if (checkVariables()!=34566543) {
	die('Variables Problem');
}
$sVar = new StdClass();
$sVar->Command = ($_POST['Command']);

if ($sVar->Command != NULL) {
	if ( sha1($_POST['psswd']) == PWD_SHA) {
		if ($sVar->Command=='Add'){
			$sVar = getVariablesAdd($sVar);
			if (!addQuerySelect($conn,$sVar)) {
				die("add Problem");
			}
		}
		elseif ($sVar->Command=='Erase'){
			$sVar = getVariablesErase($sVar);
			if (!EraseQuery($conn,$sVar->id)) {
				die("Erase Problem");
			}
		}
		elseif ($sVar->Command=='Update'){
			$sVar = getVariablesUpdate($sVar);
			$ipAddFile = fopen("myIPadd.txt", "r") or die("Unable to open file!");
			$ipAdd = fread($ipAddFile,filesize("myIPadd.txt"));
			fclose($ipAddFile);
			$baseURL =  'http://'. $ipAdd;
			$fullURL = $baseURL;
			switch ($sVar->action) {
				case "Update From List":
					$fullURL = $baseURL;
					break;
				case "Get Current Status":
					$fullURL = $baseURL. '/gpio/s';
					break;
				case "Set On":
					$fullURL = $baseURL. '/gpio/1';
					break;
				case "Set Off":
					$fullURL = $baseURL. '/gpio/0';
					break;				
			}
		}	
		else {
			die('Something not right!');
		}
	}
	else {
		die('Password is wrong!');
	}
}

printTable($conn,getTable($conn));

?>
<div> Add Query to Schedule:
	<form id="addQuery" action="dude.php" method="POST">
	<input type="hidden" name="Command" value="Add">
	  Name:<input list="names" name="myName" value="Amir">
	  <datalist id="names">
		<option value="Amir">
		<option value="Batia">
	  </datalist> <br>
	  Repeat:<input list="toRepeat" name="myRepeat" value="Once">
	  <datalist id="toRepeat">
		<option value="Once">
		<option value="Daily">
	  </datalist> <br>
	  Starting Date: :<input type="date" name="startDate" value="<?php echo date('Y-m-d'); ?>"><br>
	  Starting Hour: :<input type="Time" name="startTime" value="<?php echo date('H:i'); ?>">	<br>
	  Duration (Minutes): <input type="Number" name="Duration" value="45">	<br>
	  <button onclick='submit_Form("addQuery")'>Add</button>
	</form>
</div>

<div> Water Heater update automatically every 5 minutes, For Manual update:
<form id="updateManual" action="dude.php" method="POST">
<input type="hidden" name="Command" value="Update">
 Action:<input list="action" name="myAction" value="Update From List">
	  <datalist id="action">
		<option value="Update From List">
		<option value="Get Current Status">
		<option value="Set On">
		<option value="Set Off">		
	  </datalist> <br>
<button onclick='submit_Form("updateManual")'>Update</button>	  

<div>
<?php

if ($sVar->Command=='Update') {
	$currState = file_get_contents($fullURL);
	if ( $currState==NULL) echo 'Could Not get respond from Water Heater';
	else echo $currState;
	echo '<br>';
}

$stateFile = fopen("myState.txt", "r") or die("Unable to open file!");
$stringFromFile = fread($stateFile,filesize("myState.txt"));
fclose($stateFile);
$pos = strpos($stringFromFile, " ");
$lastTime =  date("Y-m-d H:i:s",substr($stringFromFile, 0, $pos));
 if (substr($stringFromFile, $pos+1)=="0")
	$lastState = "Off";
else
	$lastState = "On";
echo "Last auto-contact from the water heater was on ". $lastTime. " and the status was ". $lastState;


?>
</div>
<script type='text/javascript'>
	var table = document.getElementById("mainTable");   
	var rows = table.getElementsByTagName("tr");   
	for(i = 0; i < rows.length; i++){  
		if(i % 2 == 0){ 
			rows[i].style.backgroundColor = "#FFFFFF"; 
		}else{ 
			rows[i].style.backgroundColor = "#AAAAAA"; 
		}       
	}
  
function submit_Form(formID) {
	var form = document.getElementById(formID);
	var psswd = prompt("Please enter password", "pwd");
	var input = document.createElement('input');
    input.type = 'hidden';
    input.name = 'psswd';
    input.value = psswd;
    form.appendChild(input);
	//console.log(form);
	form.submit();
}
</script>


<a target="_blank" href="https://www.google.com/calendar/event?action=TEMPLATE&tmeid=YTQzNHZyMTY0cmlvMGw0YzVmNWl2NTJkOTggYW1pcmF2bmk4M0Bt&tmsrc=amiravni83%40gmail.com"><img border="0" src="https://www.google.com/calendar/images/ext/gc_button1_en.gif"></a>

</body></html>
