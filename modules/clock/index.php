<?php

date_default_timezone_set('America/Chicago');
$format = 'g:i a';

if (!empty($_GET['timezone'])) 
    date_default_timezone_set($_GET['timezone']); 

if (!empty($_GET['format'])) 
    $format = $_GET['format']; 
    
/* DATA */
$time = date($format);

/* DISPLAY */
?>

<<<<<<< HEAD:modules/clock.module.php
<div class='jumbo vertical-center'>
    <span class='clock'></span><?php echo $time; ?>
=======
<div class='mega vertical-center'>
    <span class='icon'>H</span><?php echo $time; ?>
>>>>>>> 140d0b8bc94632c3cccae35db03f904ec7fe85e5:modules/clock/index.php
</div>
