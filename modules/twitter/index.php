<?php

require_once("../../shared-libraries/twitter/twitter.class.php");

$twitter = new Twitter($_GET['consumerKey'], $_GET['consumerSecret'], $_GET['accessToken'], $_GET['accessTokenSecret']);   

try {
    if ($_GET["content"] == "replies") {
        $statuses = $twitter->load(Twitter::REPLIES);
        $label = "Tweets Mentioning @alazyreader";
    } elseif ($_GET["content"] == "me") {
        $statuses = $twitter->load(Twitter::ME);
        $label = "@alazyreader's Recent Tweets";
    }
} catch (Exception $e) {
    ?>
    <div>
        <div class="jumbo">Twitter Module</div>
        <div>Error getting tweets.</div>
        <!-- <?php $e ?> -->
    </div>
    <?php
    return;
}

?>

<div class="twitter">
	<table>
        <tr><th class="jumbo header" colspan='2'><?php echo $label; ?></th></tr>
        <?php
    	for ($i = 0; $i < $_GET['count']; $i++) {
    		$status = $statuses[$i];
        	$message = $status->text;
        	$when = $status->created_at;
        	$who = $status->user->name;?>
            <tr><td class="author"><?php echo $who; ?></td><td class="tweet"><?php echo $message; ?></td></tr>
        <?php } ?>
    </table>
</div>