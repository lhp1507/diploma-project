<?php 
	//Create connection
$conn= new mysqli("localhost", "root", "", "day");
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT * FROM times";
$result = mysqli_query($conn, $sql);
	$index = 1;
    echo "<tr>";
    echo "<th>STT</th>";
    echo "<th>Hour</th>";
    echo "<th>Minute</th>";
    echo "<th>Action</th>";
    echo "</tr>";

	if (mysqli_num_rows($result) > 0) {
	// output data of each row
        while($row = mysqli_fetch_assoc($result)) {
            echo "<tr>";
            echo "<td >". $index."</td>"; //  echo "<td >". $row["id"]."</td>";
            echo "<td >". $row["hour"]."</td>";
            echo "<td >". $row["minute"]."</td>";
            echo "<td><a href='delete.php?id=". $row["id"]."' class='btn btn-sm btn-danger' ><i>DELETE</i></a></td>";
            echo "</tr>";
        $index++;
        }
	} elseif(mysqli_num_rows($result) == 0) {
     //    echo "<tr>";
	    // echo '<div id = "notif" class=" alert alert-danger"><strong>Danger!</strong> No time data!</div>';
     //    echo "</tr>";
	}
mysqli_close($conn);

?>
  	