#!/usr/bin/perl

#svg2lvl.pl <filename.svg> <mode (0:lvl_file  1:gl_code)>

$coordScale = 0.1;

$svgFilePath = $ARGV[0];
if(@ARGV > 1){
	$outputMode = $ARGV[1];
} else {
	$outputMode = 0;
}

# read the file
open(SVGFILE, $svgFilePath);
@svgDataA = <SVGFILE>;
close(SVGFILE);
$svgData= "@svgDataA";

$lineData = "";
$polyNum = 0;

#extract polygon/point data
@pArray = multiExtractStrPart($svgData, "<path", "/>");
foreach $pathContent (@pArray) {
	updateColorInfo($pathContent);
	
	$pointsDat = extractStrPart($pathContent, "[\t\n\r ]d=\"", "\"");
	
	# get points
	# init last positions to void
	$lastX = "";
	$lastY = "";
	# assign a polygon number
	addPolygonData($polyNum);
	$polyNum++;
	
	# remove leading M
	($preDat, $pointsDat) = split(/ *M */, $pointsDat);
	# dispatch points
	@pointsList = split(/ *[LC] */,$pointsDat);
	foreach $thePointStr (@pointsList){
		@coordList = split(/[ ,]/, $thePointStr);
		# check how many coord are given (is it a flat or bezier line?)
		# the following has to be >3 instead of >2 as some paths
		# contain a "z" character at the end which is recognised as coord...
		if(@coordList > 3){
			$pointX = $coordList[4] * $coordScale;
			$pointY = - $coordList[5] * $coordScale;
		} else {
			$pointX = $coordList[0] * $coordScale;
			$pointY = - $coordList[1] * $coordScale;
		}
		
		if($outputMode == 0){
			$pointX = int($pointX * 10) / 10;
			$pointY = int($pointY * 10) / 10;
		}
		
		# add new line to line data
		if($lastX){
			addLineData($lastX, $lastY, $pointX, $pointY);
		}
		$lastX = $pointX;
		$lastY = $pointY;
	}
}

# extract rect data
@rectList = multiExtractStrPart($svgData, "<rect", "/>");
foreach $rectItem (@rectList){
	$rectWidth = int(extractStrPart($rectItem, "width=\"", "\"")) * $coordScale;
	$rectHeight = int(extractStrPart($rectItem, "width=\"", "\"")) * $coordScale;
	$rectX = int(extractStrPart($rectItem, "x=\"", "\"")) * $coordScale;
	$rectY = - int(extractStrPart($rectItem, "y=\"", "\"")) * $coordScale;
	
	updateColorInfo($rectItem);
	addRectData($rectX, $rectY, $rectWidth, $rectHeight);
}


if($outputMode == 1){
	print "glBegin(GL_LINES);\n";
}
print $lineData;
if($outputMode == 1){
	print "glEnd();\n";
}

sub addPolygonData {
	my ($pNum) = @_;
	if($outputMode == 0){
		$lineData .= "n\n$pNum\n";
	}
}

sub addLineData {
	my ($x1, $y1, $x2, $y2) = @_;
	if($outputMode == 0){
		$lineData .= "l\n$x1 $y1 $x2 $y2\n";
	} else {
		$lineData .= "glVertex2f($x1, $y1);\tglVertex2f($x2, $y2);\n";
	}
}

sub addRectData {
	my ($posx, $posy, $width, $height) = @_;
	if($outputMode == 0){
		$lineData .= "r\n$posx $posy $width $height\n";
	}
}

sub addColorStrokeData {
	my ($red, $green, $blue) = @_;
	if($outputMode == 0){
		$lineData .= "s\n$strokeRed $strokeGreen $strokeBlue\n";
	}
}

sub addColorFillData {
	my ($red, $green, $blue) = @_;
	if($outputMode == 0){
		$lineData .= "f\n$fillRed $fillGreen $fillBlue\n";
	}
}

# -------------------------
# Utility functions
# -------------------------
sub updateColorInfo {
	my ($infoStr) = @_;
	
	# get fill color information
	$fillHexColor = extractStrPart($infoStr, "fill:", ";");
	($fillRed, $fillGreen, $fillBlue) = convertSVGColor($fillHexColor);
	
	# get stoke color information
	$strokeHexColor = extractStrPart($infoStr, "stroke:", ";");
	($strokeRed, $strokeGreen, $strokeBlue) = convertSVGColor($strokeHexColor);
	
	# add color info
	addColorFillData($fillRed, $fillGreen, $fillBlue);
	addColorStrokeData($strokeRed, $strokeGreen, $strokeBlue);
}

sub extractStrPart {
	my ($theStr, $startTag, $endTag) = @_;
	
	my ($preDat, $theDat) = split(/$startTag/, $theStr);
	my ($theDat, $preDat) = split(/$endTag/, $theDat);
	
	return $theDat;
}

sub multiExtractStrPart {
	my ($theStr, $startTag, $endTag) = @_;
	
	my @inList = split(/$startTag/, $theStr);
	my @outList = ();
	my $j;
	for($j = 1; $j < @inList; $j++){
		my $listItem = $inList[$j];
		my ($listItem, $postDat) = split(/$endTag/, $listItem);
		push(@outList, $listItem);
	}
	my ($theDat, $preDat) = split(/$endTag/, $theDat);
	
	return @outList;
}

sub convertSVGColor {
	my ($theColorStr) = @_;
	
	if(substr($theColorStr,0,1) eq "#"){
		return hexColorToRGB($theColorStr);
	} elsif($theColorStr eq "none") {
		return (-1, -1, -1);
	} elsif($theColorStr eq "red") {
		return (255, 0, 0);
	} elsif($theColorStr eq "green") {
		return (0, 255, 0);
	} elsif($theColorStr eq "blue") {
		return (0, 0, 255);
	} elsif($theColorStr eq "white") {
		return (255, 255, 255);
	} elsif($theColorStr eq "black") {
		return (0, 0, 0);
	} elsif($theColorStr eq "gray") {
		return (127, 127, 127);
	} else {
		return (0, 0, 0);
	}
}

sub hexColorToRGB {
	my ($theColorStr) = @_;
	my $red = 0;
	my $green = 0;
	my $blue = 0;
	
	$red = hex(substr($theColorStr,1,2));
	$green = hex(substr($theColorStr,3,2));
	$blue = hex(substr($theColorStr,5,2));
	
	return ($red, $green, $blue);
}
