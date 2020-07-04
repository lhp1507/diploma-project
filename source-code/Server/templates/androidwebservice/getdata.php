<?php

require "dbCon.php";

$query = "SELECT * FROM times ORDER BY hour, minute";

$data = mysqli_query($connect, $query);

class Times{
	function Times($id, $hour, $minute, $seconds){
		$this->id = $id;
		$this->hour = $hour;
		$this->minute = $minute;
		$this->seconds = $seconds;
	}
}

$listTimes = array();

while ($row = mysqli_fetch_assoc($data)) {
	array_push($listTimes, new Times($row['id'], $row['hour'], $row['minute'], $row['seconds']));
}

echo json_encode($listTimes);

?>