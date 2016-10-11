<?php

/* DATA */
$data = array(
  array('TRANSMIT',     'Blurb', 'Blarb', 'Z@statuspanic, H@statuspanic'),
  array('UNISON',       'Blurb', 'Blarb', 'Z@statuspanic, L@statuspanic, I@statuspanic'),
  array('CODA',         'Blurb', 'Blarb', 'Z@statuspanic, H@statuspanic, P@statuspanic'),
  array('OTHER THING',  'Blurb', 'Blarb', 'G@statuspanic, L@statuspanic'),
  array('EXCITING',     'Blurb', 'Blarb', 'L@statuspanic, G@statuspanic, I@statuspanic')
);

/* DISPLAY */

?>

<div>
    <table border='0' width='100%' cellpadding='0' cellspacing='10'>
    <?php
    $count = 0;
    foreach($data as $row) {
        $class = ($count % 2 == 1 ? " class='alt'" : '');
        echo "<tr$class>";
        for($j = 0; $j < count($row); $j++) {
            if ($j!=3) {
                echo "<td class='cell_$j'>$row[$j]</td>";
            } else {
                $gravatar = ''; 
                $array = preg_split('/,/', $row[$j], -1, PREG_SPLIT_NO_EMPTY);
                foreach ($array as $email) {
                    $gravatar .= '<img src="http://www.gravatar.com/avatar.php?gravatar_id='. md5($email) .'&s=40&d=monsterid"> ';            
                }
                echo "<td class='cell_$j'>$gravatar</td>";
            }

        }
        echo '</tr>';
        $count++;
    }
    
    ?>
    
    </table>
</div>
