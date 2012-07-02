<?php 
require "../../head.inc"; 
require "../directories.inc"; 

printf('<?xml version="1.0" ?>');?> <?php if (!$do_wml) { ?>

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
	<head>
		<title>birkler.se</title>
		<link href="../../birkler.css" rel="stylesheet" type="text/css" />
	</head>
	<body>
		<h2>Projects</h2>
		<?php DirectoryList("."); ?>
	</body>
</html>
<?php } else {?>
<wml>
	<card id="welcome" title="Welcome">
		<p>
			Sorry, your browser does not seem to support xhtml, only wml.
		</p>
	</card>
</wml>
<?php } ?>
