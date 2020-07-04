<?php 
// $submit = $_POST['sub'];
if (!empty($_POST['sub'])) { // empty() To check if it is available
	# code..
	// echo shell_exec("python/var/www/html/test/php_var_to_python/echo.py 'submit' ");
	$hour= $_POST['hour'];
	$minute= $_POST['minute'];
	$seconds = $hour*3600+$minute*60;
	
	$host= "localhost";
	$dbUserName= "root"; //Localhost thì đây là root
	$dbPassword= "";
	$dbName= "day";    //Tên của cơ sở dữ liệu

	//Create connection
	$conn= new mysqli($host, $dbUserName, $dbPassword, $dbName);

	// Check connection
	if ($conn->connect_error) {
	    die("Connection failed: " . $conn->connect_error);
	}

	$result = $conn->query("SELECT id FROM times WHERE seconds = '$seconds' ");
	if($result->num_rows == 0){
		// Nếu không tồn tại giây giống với cái nhập vào 

		// $sql = "INSERT INTO times (hour, minute, seconds) VALUES ('$hour', '$minute', '$hour'*3600+'$minute'*60)";
		$sql = "INSERT INTO times (hour, minute, seconds) VALUES ('$hour', '$minute', '$seconds')";

		if ($conn->query($sql) === TRUE) {
	
			// echo "alert('New record created successfully')";  // showing an alert box.
					   
		} else {	

		    echo "Error: " . $sql . "<br>" . $conn->error;
		}
	}else{
		// Nếu có tồn tại giá trị giống với cái nhập vào 

		echo '<script language="javascript">';
		echo 'alert("Your remind has exist!")';
		echo '</script>';
	}

	

	$conn->close();
	// header('Location: index.html'); //Quay về trang chủ sau khi thực hiện xong
	echo '<script language="javascript">';
	echo "window.location.href = 'index.html';";
	echo '</script>';	

}
?>
