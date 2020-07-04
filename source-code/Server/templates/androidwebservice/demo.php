<?php
	class Times{
		function Times($id, $hour, $minute, $seconds){
			$this->id = $id;
			$this->hour = $hour;
			$this->minute = $minute;
			$this->seconds = $seconds;
		}
	}

	$listTimes = array();

	array_push($listTimes, new Times(1, 14, 16, 14 * 3600 + 16 * 60));
	array_push($listTimes, new Times(2, 15, 16, 15 * 3600 + 16 * 60));
	array_push($listTimes, new Times(3, 16, 16, 16 * 3600 + 16 * 60));
	array_push($listTimes, new Times(4, 17, 16, 17 * 3600 + 16 * 60));

	echo json_encode($listTimes);
?>