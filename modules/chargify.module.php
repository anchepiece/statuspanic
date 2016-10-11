<?php

$domain = $_GET['domain'];
$api_key = $_GET['api_key'];
$data = array(
  array('', 'Today', 'Month', 'Year', 'Total')
);

if( $domain && $api_key ){
  
  $stats_url = "https://$api_key:x@$domain.chargify.com/stats.json";

  $json = file_get_contents( $stats_url );

  $stats = json_decode( $json );
  $stats = $stats->{'stats'};
 
  $data[] = array('Subscriptions', $stats->subscriptions_today, '', '', $stats->total_subscriptions);
  $data[] = array('Revenue', $stats->revenue_today, $stats->revenue_this_month, $stats->revenue_this_year, $stats->total_revenue);

} else {
  error_log( "Are api_key and domain set in config.json? I got [$api_key] and [$domain]." );
}


?>
    <div>
        <table border='0' width='100%' cellpadding='0' cellspacing='10'>
        <?php
        $count = 0;
        foreach($data as $row) {
            $class = ($count % 2 == 1 ? " class='alt'" : '');
            echo "<tr$class>";
            for($j = 0; $j < count($row); $j++) {
                    echo "<td>$row[$j]</td>";

            }
            echo '</tr>';
            $count++;
        }

        ?>

        </table>
    </div>