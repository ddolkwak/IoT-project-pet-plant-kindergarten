function XHR_write(data){
        var xhr = new XMLHttpRequest();

        xhr.open("GET","remote_con.php?" + data);
        xhr.send();
}

function XHR_read(data){
        var xhr = new XMLHttpRequest();

        xhr.open("GET","remote_con.php?" + data, false);
        xhr.send();
		
	return xhr.responseText;
}

var light_check = 0;

function repeat(){
	//GAS();
	
	if (light_check == 1)
		LIGHT();
	//PIR(); US(); TEMP(); LIGHT(); SOUND(); HUMI(); //TEMP_HUMI(); //VR(); SW(); TOUCH(); PD(); BUMP(); IRO(); MERC(); TILT(); FLAME(); REED();
}
setInterval(function(){repeat();},1000);

//--------------------------------------------------------//

function LED_ON(){
	XHR_write('LEDON');
	document.LED.src='img/led_on.png';
}

function LED_OFF(){
	XHR_write('LEDOFF');
	
	document.LED.src='img/led_off.png';
}

function DCM_ON(){
	XHR_write('DCMON');
	
	document.DCM.src='img/ventilation_on.png';
}

function DCM_OFF(){
	XHR_write('DCMOFF');
	
	document.DCM.src='img/ventilation_off.png';
}


function CHECK_ALL(){
	document.CHECKALL.src = "img/auto_on.png";
	
	setTimeout(function(){
		var val = XHR_read('CHECKALL');
		console.log(val);
		document.CHECKALL.src = 'img/auto_off.png';
		
		var tmpArray = val.split(" ");
		var temp = tmpArray[0];
		var humi = tmpArray[1];
		var soil = tmpArray[2];
		var dust = tmpArray[3];
		
		document.getElementById("TEMP_val").value = temp + "'C";
		document.getElementById("HUMI_val").value = humi + "%";
		document.getElementById("SM_val").value = Math.round(soil/40.96) + "%";
		document.getElementById("DUST_val").value = dust;
		
		if (temp < 14) document.EMOJI.src = 'img/cold.png';
		else if (temp > 28) document.EMOJI.src = 'img/hot.png';
		else document.EMOJI.src = 'img/good.png';
		
		if (soil < 1200) document.EMOJI.src = 'img/bad.png';
		if (dust > 400) document.EMOJI.src = 'img/dustbad.png';
		
		
	}, 100);
}

function TEMP_HUMI() {
	document.TEMPHUMI.src = 'img/dht_on.png';
	
	setTimeout(function(){
		var val = XHR_read('CHECKALL');
		console.log(val);
		document.getElementById("TEMP_val").value = val;
		document.TEMPHUMI.src = 'img/dht_off.png';
	}, 100);
}

function DUST_ON() {
	document.DUST.src = 'img/dust_on.png';

	setTimeout(function () {
		var val = XHR_read('DUST');
		document.getElementById("DUST_val").value = val;
		document.DUST.src = 'img/dust_off.png';
		
		if (val > 400) document.EMOJI.src = 'img/dustbad.png';
		else document.EMOJI.src = 'img/good.png';
	}, 100);
}

function SM_ON() {		
	document.SM.src = 'img/soil_on.png';

	setTimeout(function () {
		var val = XHR_read('SM');
		document.getElementById("SM_val").value = Math.round(val/40.96) + "%";
		document.SM.src = 'img/soil_off.png';
		
		if (val < 1200) document.EMOJI.src = 'img/bad.png';
		else document.EMOJI.src = 'img/good.png';
	}, 100);
}

function GAS(){
	var val=XHR_read('GAS');
	
	console.log(val);
	if(val>=1000){
		console.log("Warning");
	}
}

function LIGHT(){
	var val=XHR_read('LIGHT');
}

function LIGHT_ON(){
	light_check = 1;
}

function LIGHT_OFF(){
	light_check = 0;
}

/*
function PIR(){
	var val=XHR_read('PIR');
	
	if(val==1){
		document.PIR.src='soilON.png';
	}else{
		document.PIR.src='soilOFF.png';
	}
}
function US(){
	var val=XHR_read('US');
	
	if(val>=100){
		document.US.src='img/ultrasonic3.png';
	}else if(val>=50){
		document.US.src='img/ultrasonic2.png';
	}else{
		document.US.src='img/ultrasonic1.png';
	}
	
	document.getElementById("US_val").value=val+"cm";
}
*/	

/*


function SOUND(){
	var val=XHR_read('SOUND');
	
	document.getElementById('SOUND_val').value=val;
	document.SOUND.style.opacity=val/2048/(4/5)+0.2;
}
*/
	
/*
function VR(){
	var val=XHR_read('VR');
	
	document.getElementById('VR_val').value=val;
	document.VR.style='transform : rotate(' + (val/4095*90-90) + 'deg);';
}	

function SW(){
	var val=XHR_read('SW');
	
	if(val==0){
		document.SW.src='img/sw_on.png';
	}else{
		document.SW.src='img/sw_off.png';
	}
}

function TOUCH(){
	var val=XHR_read('TOUCH');
	
	if(val==1){
		document.TOUCH.src='img/touch_on.png';
	}else{
		document.TOUCH.src='img/touch_off.png';
	}
}

function PD(){
	var val=XHR_read('PD');
	
	if(val==1){
		document.PD.src='img/pd_on.png';
	}else{
		document.PD.src='img/pd_off.png';
	}
}

function BUMP(){
	var val=XHR_read('BUMP');
	
	if(val==1){
		document.BUMP.src='img/bump_on.png';
	}else{
		document.BUMP.src='img/bump_off.png';
	}
}

function IRO(){
	var val=XHR_read('IRO');
	
	if(val==0){
		document.IRO.src='img/iro_on.png';
	}else{
		document.IRO.src='img/iro_off.png';
	}
}

function LASERON(){
	XHR_write('LASERON');
	
	document.LASER.src='img/laser_on.png';
}

function LASEROFF(){
	XHR_write('LASEROFF');
	
	document.LASER.src='img/laser_off.png';
}

function MERC(){
	var val=XHR_read('MERC');
	
	if(val==1){
		document.MERC.src='img/mercury_on.png';
	}else{
		document.MERC.src='img/mercury_off.png';
	}
}

function TILT(){
	var val=XHR_read('TILT');
	
	if(val==1){
		document.TILT.src='img/tilt_on.png';
	}else{
		document.TILT.src='img/tilt_off.png';
	}
}

function FLAME(){
	var val=XHR_read('FLAME');
	
	if(val==1){
		document.FLAME.src='img/flame_on.png';
	}else{
		document.FLAME.src='img/flame_off.png';
	}
}

function REED(){
	var val=XHR_read('REED');
	
	if(val==1){
		document.REED.src='img/reed_on.png';
	}else{
		document.REED.src='img/reed_off.png';
	}
}

function BUZZERON(){
	XHR_write('BUZZERON');
	
	document.BUZZER.src='img/buzzer_on.png';
}

function BUZZEROFF(){
	XHR_write('BUZZEROFF');
	
	document.BUZZER.src='img/buzzer_off.png';
}
*/
