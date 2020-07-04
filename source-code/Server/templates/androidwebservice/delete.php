<?php
require "dbCon.php";

$id = $_POST['id'];

$query = "DELETE FROM times WHERE id = '$id'";

if(mysqli_query($connect, $query)){
	echo "success";
}else{
	echo "error";
}

?>