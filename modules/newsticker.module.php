<div class="newsticker" style="height: 2em; overflow: hidden;">
<?php

$datadir=$_GET['datadir'];
$itemsArg=$_GET['items'];
$items = explode (",", $itemsArg);

foreach ($items as $item) {
  include($datadir . "/news_" . $item . ".php");
}
?>
</div>


<script type="text/javascript">
$(document).ready(function(){

    $('.newsticker div').each(function(){
        $(this).css('display','none');
    });
    $('.newsticker div:first').css('display','block');

    function news_displayNext(){
        current = $('.newsticker div:first');
        $('.newsticker div:first').fadeOut('slow', function() {
                $(this).remove();
                news_queueNext(current);
            });
    }

    function news_queueNext(e){
        last = e.clone();
        $(last).css('display', 'none');
        $('.newsticker').append(last);
        $('.newsticker div:first').fadeIn('slow');
    }

    interval = setInterval(news_displayNext, 8000);
});
</script>

