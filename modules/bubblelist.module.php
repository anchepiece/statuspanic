<?php

$items = array(
    // 'bubble' => 'line'
    '4|orange'  => '1:02',
    '9|red'     => '9 min',
    '14|red'    => '5 min',
    '19|red'    => '11 min',
    '20|orange' => '3:55'
);

?>

<ul>
    <?php foreach($items as $bubble => $line) { 
        $bubble = explode('|', $bubble);
        $color  = $bubble[1];
        $bubble = $bubble[0];
        ?>
        <li>
            <span class='<?php echo $color ?> bubble'>
                <span class='display'><?php echo $bubble ?></span>
            </span>
            <span class='content'><?php echo $line ?></span>
        </li>
    <?php } ?>
</ul>
