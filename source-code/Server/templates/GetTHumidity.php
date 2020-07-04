<?php 	
$conn = mysqli_connect("localhost", "root", "", "day");
$result = mysqli_query($conn, "SELECT humidity FROM humidity WHERE id = (SELECT MAX(id) FROM humidity)");

$data = array();
while ($row = mysqli_fetch_object($result))
{
    array_push($data, $row);
}

echo json_encode($data);
exit();
?>