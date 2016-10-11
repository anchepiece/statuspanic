<?php

require_once("../../shared-libraries/magpie/rss_fetch.inc");
$feed = $_GET['feed'];
$rss = fetch_rss( $feed );
$feed_name = $rss->channel['title'];


for ($i = 0; $i < $_GET['stories']; $i++) { 
	$item = $rss->items[$i];
    $title = $item['title'];
    $items .= "<span>$title</span><br/>";
}

?>

<div>
    <div class="padding">
    	<span class="jumbo"><?php echo $feed_name ?></span>
	    <div>
	        <?php echo $items ?>
	    </div>
	</div>
</div>