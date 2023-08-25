<html>
<head>
<title><?php echo getcwd(); ?></title>
<style type='text/css'>
  body
  {
  font-family: "Candara", sans-serif;
  font-size: 12pt;
  line-height: 10.5pt;
  }

  div.pic
  { 
  display: block;
  float: left;
  background-color: white;
  border: 1px solid #ccc;
  padding: 2px;
  text-align: center;
  margin: 2px 10px 10px 2px; 
  }
</style>
</head>
<body>
<h2><a name="plots">Plots</a></h2>
<div>
<p><form>Filter: <input type="text" name="match" size="30" value="" /><input type="Submit" value="Go" /></form></p>

<?php
$remainder = array();
if ($_GET['noplots']) {
  print "Plots will not be displayed.\n";
} else {
  //$other_exts = array('.pdf', '.cxx', '.eps');
  foreach (glob("*") as $filename) {
    // if (!preg_match('/.*\.png$/', $filename) && !preg_match('/.*\.gif.*$/', $filename)) {
      if (!preg_match('/.*\.svg$/', $filename)) {
      if (!preg_match('/.*\.php.*$/', $filename)) {
	array_push($remainder, $filename);
      }
      continue;
    }
    if (isset($_GET['match']) && !fnmatch('*'.$_GET['match'].'*', $filename)) continue;
    print "<div class='pic'>\n";
    print "<h3><a href=\"$filename\" target=\"_blank\">$filename</a></h3>";
    // print "<a href=\"$filename\" target=\"_blank\"><img src=\"$filename\" style=\"border: none; width: 300px; \" target=\"_blank\"></a>";
    print "<a href=\"$filename\" target=\"_blank\"><object data=\"$filename\" width=\"300\"> </object></a>";
    // print "<a href=\"$filename\"><img src=\"$filename\" style=\"border: none; width: 300px; \"></a>";
    //$others = array();
    //foreach ($other_exts as $ex) {
    //    if (file_exists(str_replace('.png', $ex, $filename))) {
    //        array_push($others, "<a class=\"file\" href=\"".str_replace('.png', $ex, $filename)."\">[" . $ex . "]</a>");
    //    }
    //}
    //if ($others) print "<p>Also as ".implode(', ',$others)."</p>";
    print "</div>";
  }
 }
?>
</div>
<div style="display: block; clear:both;">
  <h2><a name="files">&nbsp;<P>Files</a></h2>
<ul>
<?
foreach ($remainder as $filename) {
  // print "<li><a href=\"$filename\" target=\"_blank\">$filename</a></li>";
  print "<li><a href=\"$filename\" target=\"_blank\">$filename</a></li>";
}
?>
</ul>
</div>
</body>
</html>
