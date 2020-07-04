<?php 

	$id= $_GET['id'];
	echo $id;
	//Create connection
	$conn= new mysqli("localhost", "root", "", "day");
	// Check connection
	if ($conn->connect_error) {
		die("Connection failed: " . $conn->connect_error);
	}
	// sql to delete a record
	$sql = "DELETE FROM times WHERE id= '$id'";

	if ($conn->query($sql) === TRUE) {
		echo '<script type="text/javascript">';
		echo ' alert("Record deleted successfully")';  //not showing an alert box.
		echo '</script>';
	    // echo "Record deleted successfully";
	} else {
	    echo "Error deleting record: " . $conn->error;
	}
	mysqli_close($conn);

	header('Location: index.html'); //Quay về trang chủ sau khi thực hiện xong 

 ?>

