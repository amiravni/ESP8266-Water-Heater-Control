<?php
include_once "init.php";
date_default_timezone_set('Asia/Jerusalem');

function startConnection() {
	$servername = "localhost";
	$username = USERNAME;
	$password = PWD_DB;
	$dbname = NAME_DB;

	// Create connection
	$connTMP = new mysqli($servername, $username, $password, $dbname);
	// Check connection
	if ($connTMP->connect_error) {
		die("Connection failed: " . $connTMP->connect_error);
	}

	return $connTMP;
}



function deleteTable($conn) {
	$sql = "DROP TABLE ".NAME_DB_TOUSE."";
	return $conn->query($sql);
}

function createTable($conn) {
	$sql = "CREATE TABLE IF NOT EXISTS ".NAME_DB_TOUSE." (
	id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY, 
	name VARCHAR(30) NOT NULL,
	toRepeat VARCHAR(30) NOT NULL,
	startTime TIMESTAMP,
	endTime TIMESTAMP
	)";
	return $conn->query($sql);
}

function getTable($conn) {
	$sql = "SELECT id, name, toRepeat, startTime,endTime FROM ".NAME_DB_TOUSE." ORDER BY startTime";
	return $conn->query($sql);

}

function EraseQuery($conn,$id){
	$sql = "DELETE FROM ".NAME_DB_TOUSE." WHERE id=". $id;
	return ($conn->query($sql));
}

function isOutOfDate($row) {
	return (strtotime($row["endTime"]) - strtotime(date("Y-m-d H:i:s")) < 0);
} 
?>