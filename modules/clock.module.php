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

<div class='jumbo vertical-center'>
    <span class='clock'></span><?php echo $time; ?>
</div>
