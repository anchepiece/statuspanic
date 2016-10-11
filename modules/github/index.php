<?php

require_once("../../shared-libraries/magpie/rss_fetch.inc");

if (empty($_GET['branch'])) {
    $branch = "master";
} else {
    $branch = $_GET['branch'];
}

try {
    $repo = fetch_rss( $_GET['repo']."commits/".$branch.".atom" );    
} catch (Exception $e) {
    ?>
    <div>
        <div class="jumbo">Github Module</div>
        <div>Error getting commits.</div>
        <!-- <?php $e ?> -->
    </div>
    <?php
    return;
}

$feed_name = $repo->channel['title'];

?>

<div class="github">
	<span class="jumbo"><?php echo $feed_name ?></span>
    <table border='0' width='100%' cellpadding='0' cellspacing='10'>
    <?php
	for ($i = 0; $i < $_GET['commits']; $i++) {
		$item = $repo->items[$i];
    	$title = $item['title'];
    	$author = $item['author'];
    	$avatar = $item['media']['thumbnail@url'];?>
        <tr>
        <td style='width: 70%;'><div style='overflow: hidden; width: 600px; white-space:nowrap;'><?php echo $title; ?></div></td>
        <td style='width: 30%;'><img src='<?php echo $avatar; ?>'/>&nbsp;<?php echo $author; ?></td>
        </tr>
    <?php } ?>
    </table>
</div>