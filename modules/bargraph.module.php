<?php

class Bar {
    function __construct($name, $height, $remaining) {
        $this->name = $name;
        $this->height = $height;
        $this->remaining = $remaining;
    }
}

/* DATA */
$bars = array();
$bars[] = new Bar('TRANSMIT',  18,   1); 
$bars[] = new Bar('CODA',      29,   3); 
$bars[] = new Bar('UNISON',    48,   3); 
$bars[] = new Bar('CANDYBAR',   8,   2); 
$max = rand(3, 100);
$bars[] = new Bar('GENERAL', $max,   0);
$bars[] = new Bar('SECRETS',   15,   2);


/* DISPLAY */
$max_height = 0;
foreach($bars as $bar) {
    if ($bar->height > $max_height)
        $max_height = $bar->height;
}

// change these
$max_bar_width = 300;
$default_padding = 12;

// don't change these
$total_outer = ($default_padding * 2); // (paddings + borders)
$max_width = $_GET['width'];
$num_bars = count($bars);
$bar_width = floor(min($max_bar_width, ($max_width - ($total_outer * $num_bars)) / $num_bars));
$final_padding = max($default_padding, ($max_width - (($bar_width + $total_outer) * $num_bars)) / $num_bars / 2);
?>

<div>
  <div class="bars">
<?php for($j = 0; $j < count($bars); $j++) {
    $bar = $bars[$j];
    $count = $j + 1;
    $bar_height =  ($bar->height / $max_height) * $_GET['height'];
    $bar_height = floor($bar_height);
    $top_offset = $_GET['height'] - $bar_height;

    
?>
    <div class='bar' style='margin-top: <?php echo $top_offset . 'px; width: ' .
                            $bar_width . 'px; padding: 0 ' . $final_padding . 'px;' ?>'>
        <div class='header'><?php echo '<span class="total">'. $bar->height .'</span> / <span class="remaining">'. $bar->remaining .'</span>'; ?></div>
        <div class='view' id='bar_<?php echo $count ?>' style='height: <?php echo $bar_height; ?>px;'></div>
    </div>
<?php } ?>
  </div>
<?php for($j = 0; $j < count($bars); $j++) {
    $bar = $bars[$j]; 
?>  
    <div class='bar-title' style='width: <?php echo $bar_width . 'px; padding: 0 ' . $final_padding . 'px;' ?>'>
        <span class='title'><?php echo $bar->name ?></span>
    </div>
<?php } ?>        
<div style='clear:both'></div>
</div>
