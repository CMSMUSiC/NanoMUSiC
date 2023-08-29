<html>
<head>
<title><?php echo getcwd(); ?></title>
<style type='text/css'>
  /* body
  {
  font-family: sans-serif;
  font-size: 12pt;
  line-height: 10.5pt;
  } */

  div.pic
  { 
  display: inline;
  float: left;
  background-color: white;
  border: 1px solid #ccc;
  padding: 2px;
  text-align: center;
  margin: 2px 10px 10px 2px; 
  }
</style>
    
    <!-- Required meta tags -->
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <!-- Bootstrap CSS -->
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC" crossorigin="anonymous">

    
</head>
<body>


<h2><a name="plots">NanoMUSiC Plots</a></h2>
<div>
<!-- <p><form>Filter: <input type="text" name="match" size="30" value="" /><input type="Submit" value="Go" /></form></p> -->

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
    // if (isset($_GET['match']) && !fnmatch('*'.$_GET['match'].'*', $filename)) continue;
    print "<div class='pic'>\n";
    print "<h3><a href=\"" . str_replace(".svg", ".pdf", $filename). "\" target=\"_blank\">" . str_replace(".svg", "", $filename) . "</a></h3>";

    // print "<a href=\"$filename\" target=\"_blank\"><img src=\"$filename\" style=\"border: none; width: 300px; \" target=\"_blank\"></a>";
    print "<a href=\"$filename\" target=\"_blank\"><object data=\"$filename\" width=\"300\"> </object></a>";

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

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js" integrity="sha384-geWF76RCwLtnZ8qwWowPQNguL3RmwHVBC9FhGdlKrxdiJJigb/j/68SIy3Te4Bkz" crossorigin="anonymous"></script>

</body>
</html>
