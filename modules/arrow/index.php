<?php

/* DATA */

$num = rand(-99, 99);

if (rand(0,4) == 0) {
    $num = 0;
}

if (!empty($_GET['value'])) 
    $num = $_GET['value'];

/* DISPLAY */
if ($num > 0) {
    $class = 'uparrow';
} elseif ($num < 0) {
    $class = 'downarrow';
} else {
    $class = 'zero-block';
}
?>

<div>    
    <span class='<?php echo $class ?>' id='arrow_icon'></span>
    <span class='mega'><?php echo $num ?>%</span>
</div>
