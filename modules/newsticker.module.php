<div class="newsticker" style="height: 2em; overflow: hidden;">
  <div class='jumbo'>Created by <a href='http://allynbauer.com'>Allyn Bauer</a></div>
  <div class='jumbo'>Get it at <a href='http://github.com/ajb/statuspanic'>GitHub</a></div>
  <div class='jumbo'>Today: <strong><?php echo date('Y-m-d'); ?></strong></div>
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

