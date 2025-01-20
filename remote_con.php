<?php
	if(isset($_GET['LEDON'])){
		$value = shell_exec("/home/pi/PHP_LEDON");
		echo $value;
	}else if(isset($_GET['LEDOFF'])){
		$value = shell_exec("/home/pi/PHP_LEDOFF");
		echo $value;
	}else if(isset($_GET['DCMON'])){
                $value = shell_exec("/home/pi/PHP_DCMON");
                echo $value;
	}else if(isset($_GET['DCMOFF'])){
                $value = shell_exec("/home/pi/PHP_DCMOFF");
                echo $value;
	}else if(isset($_GET['CHECKALL'])){                     
                $value = shell_exec("/home/pi/PHP_CheckEnvironment");
                echo $value;
	}else if(isset($_GET['TEMP_HUMI'])){                     
                $value = shell_exec("/home/pi/PHP_CheckEnvironment");
                echo $value;
	}else if(isset($_GET['DUST'])){
                $value = shell_exec("/home/pi/PHP_DUST");
                echo $value;
    }else if(isset($_GET['SM'])){
                $value = shell_exec("/home/pi/PHP_SM");
                echo $value;
    }else if(isset($_GET['GAS'])){                        
                $value = shell_exec("/home/pi/PHP_GAS");
                echo $value;
    }else if(isset($_GET['LIGHT'])){
                $value = shell_exec("/home/pi/PHP_LIGHT");
                echo $value;
    }
?>
