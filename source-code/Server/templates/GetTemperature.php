<?php 	
$conn = mysqli_connect("localhost", "root", "", "day");
$result = mysqli_query($conn, "SELECT temperature FROM temperature WHERE id = (SELECT MAX(id) FROM temperature)");

$data = array();
while ($row = mysqli_fetch_object($result))
{
    array_push($data, $row);
}

echo json_encode($data);
exit();
?>