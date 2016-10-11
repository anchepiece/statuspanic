function render_module(name, args, firstrun) {
  $('#' + name).load('modules/' + name + '.module.php?' + args , function() {
    if (firstrun==1)
        module_init (name);
    });
}

function activate_module(name, seconds, args) {
  render_module(name, args, 1);
  if (seconds > 0) {
    setInterval("render_module('"+name+"', '"+args+"', '0')", (seconds * 1000));
  }
}

$(document).ready(function() {
  $('.middle').each(function(id, val) {
    var outer_height = $(val).height();
    var inner_height = $(val).children().first().height();
    var buffer = (outer_height - inner_height) / 2;
    var SEL = '#' + $(val).attr('id') + '>div';
    $(SEL).css('marginTop', buffer);
    $(SEL).css('marginBottom', buffer);
  });
});

function module_init (name) {
    switch (name) {
        case 'stockticker':
            stockticker_init();
	        break;
	    default:
	        break;
    }
}

// stockticker global vars
//var ticker = 0;
var left = 0;
function stockticker_init() {
    ticker = $('.stockticker');
	ticker.css('position','absolute');
	ticker.css('display','block');
	ticker.css('opacity','0');
    ticker.animate({ opacity: 1 }, 6000);
    startTicker();
}

function startTicker() {
    ticker = $('.stockticker');
	ticker.css('position','absolute');
	ticker.css('display','block');

    var shiftLeftAt = ticker.children().get(0).offsetWidth ;
    ticker.width(screen.availWidth + 2 * shiftLeftAt);

    $('.stockticker div').each(function(){
        $(this).css('float','left');
        $(this).css('padding-left','2em');
        $(this).css('display','block');
    });

    left -= 1;
    if(left <= shiftLeftAt * -1) {
        left = 0;
        ticker.append(ticker.children().get(0));
		ticker.remove(ticker.children().get(0));
        shiftLeftAt = ticker.children().get(0).offsetWidth;
    }
    ticker.css('left', left + 'px');
	setTimeout(arguments.callee, 30);
}

