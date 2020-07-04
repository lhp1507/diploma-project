<?php
require "dbCon.php";

$id = $_POST['id'];
$hour = $_POST['hour'];
$minute = $_POST['minute'];
$seconds = $_POST['seconds'];

$query = "UPDATE times SET hour = '$hour', minute = '$minute', seconds = '$seconds' WHERE id = '$id'";

if(mysqli_query($connect, $query)){
	echo "success";
}else{
	echo "error";
}

?>