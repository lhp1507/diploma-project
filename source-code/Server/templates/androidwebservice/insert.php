<?php
// require "dbCon.php";

// $hour = $_POST['hour'];
// $minute = $_POST['minute'];
// $seconds = $_POST['seconds'];


// 	$query = "INSERT INTO times VALUES(null, '$hour', '$minute', '$seconds')";

// 	if(mysqli_query($connect, $query)){
// 		echo "success";
// 	}else{
// 		echo "error";
// 	}

?>

<?php 
$hour = $_POST['hour'];
$minute = $_POST['minute'];
$seconds = $_POST['seconds'];

$conn= new mysqli("localhost", "root", "", "day");
if ($conn->connect_error) {
	
	die("Connection failed: " . $conn->connect_error);

}else{
	
	$result = $conn->query("SELECT id FROM times WHERE seconds = '$seconds' ");
	if($result->num_rows == 0){
		$sql = "INSERT INTO times (hour, minute, seconds) VALUES ('$hour', '$minute', '$seconds')";
		if ($conn->query($sql) === TRUE) {
			echo "success";

		}else {	

		echo "error";
		echo "Error: " . $sql . "<br>" . $conn->error;
		}
	}else{
		echo "Has existed";
	}
}


 ?>