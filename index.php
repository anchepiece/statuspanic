<?php
define('CONFIG', 'config.json');

// FIRST: read in the configuration
$config = fopen(CONFIG, 'r');
$data = fread($config, filesize(CONFIG));
$data = json_decode($data, true);

if (!$data) die('JSON syntax error in "'.CONFIG.'"');

if ($data['rotate'] === 'left') {
    $rotate = '-webkit-transform: rotate(-90deg);';
}
elseif ($data['rotate'] === 'right') {
    $rotate = '-webkit-transform: rotate(90deg);';
} elseif ($data['rotate'] === 'flip') {
    $rotate = '-webkit-transform: rotate(180deg);';
} else {
    $rotate = FALSE;
}

$width = (isset($data['width']) ? $data['width'] . 'px' : '100%');

function render($module) {
    $argstr = array();
    $args = isset($module['args']) ? $module['args'] : array();
    $args['width'] = $module['width'];
    foreach($args as $key => $val) {
        $argstr[] = "$key=" . urlencode($val);
    }
    $argstr = "'" . implode("&", $argstr) . "'";
    
    $style = "width: {$module['width']}px;";
    if (isset($module['height'])) $style .= " height: {$module['height']}px";
    echo "<div class='module".(isset($module['class'])?' '.$module['class']:'')."' id='".$module['name']."' style='$style'></div>\n";
    echo "\t<script type='text/javascript'>activate_module('".$module['name']."', ".$module['update'].", $argstr);</script>\n\n";
}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html lang="en-US" xml:lang="en-US" xmlns="http://www.w3.org/1999/xhtml">
<head>
    <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
    <title><?php echo (isset($data->title) ? $data->title : 'statuspanic generic status board') ?></title>
    <link rel='stylesheet' type='text/css' href='resources/panic.css' />
    <script type='text/javascript' src='resources/jquery.js'></script>
    <script type='text/javascript' src='resources/board.js'></script>
    <style type='text/css'>
        #board {
        <?php if ($rotate) {
            echo $rotate;
            echo "height: $width;";
        } else {
            echo "width: $width;";
        } ?>
    }
    </style>
</head>
<body>
    <div id='board'>
        <?php 
        foreach($data['modules'] as $module)
            render($module);
        ?>
    </div>
</body>
</html>
