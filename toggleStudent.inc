<?php
// ==================================================================
//  Author: Robert Joseph (dart@vizmotion.com)
//	 Web: http://wiki.bssd.org/index.php/DART_System
// ==================================================================

include_once("lib/functions_gs.inc");
include_once("ReportPrivileges.php");

$cellwidth = 60;
$studentID = (!isset($currentStudentID) || $currentStudentID == '') ? $_GET['p1'] : $currentStudentID;
$student = getStudentInfo($studentID);

$_SESSION['currentStudentID'] = $studentID;
$_SESSION['currentStudentName'] = $student['fname'] . ' ' . $student['lname'];

$site = $student['site'];
$g1 = $student['grade'];

$siteEqual = isset($currentMySite) && $currentMySite == $site;
$idEqual = $currentAffiliatedID == $studentID;
//$idEqual = 0;
$A2Priv = Privilege(A2);
$A3Priv = Privilege(A3);

if(!isset($currentStudentID) || $currentStudentID == '') {
	$edit = true;
	$Display = checkStudent($idEqual, $siteEqual, $A2Priv, $A3Priv);
	if ($Display == 'no') {
		$type = "denied toggleStudent";
		$notes = "page=toggleStudent;site=$site;studentID=$studentID;currentSite=$currentMySite;idEqual=$idEqual;siteEqual=$siteEqual;A2Priv=$A2Priv;A3Priv=$A3Priv";
		//echo("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=index.php?cmd=noPrivilege&type=$type&notes=$notes\"> ");
	}
} else {
	$edit = false;
	$Display = 'student';
}

$studentChanges = getStudentChanges2($studentID);

$studentName = $student['fname'] . " " . $student['lname'];

//$reportPrivileges = new ReportPrivileges($idEqual, $siteEqual, $studentID, $edit, $CurrentYear);
//$reportLinks = $reportPrivileges->getLinks();
//$reportNames = $reportPrivileges->getNames();
//array_unshift($reportLinks, "");
//array_unshift($reportNames, "None");

$summary = getSummaryInfo();

$subjects = getStudentLevels($studentID);

$subjectMenu = array(
		"" => "Choose One",
		"#top_anchor" => "Top of Page"
);
$subjectSelected = "";

foreach ($subjects as $rec) {
	$anchor = $rec['subject'];
	$subjectMenu["#$anchor"] = $anchor;
}
if($Display == 'edit') {
	$subjectMenu["#save_changes"] = "Save All Changes";
}

$sql = "SELECT gradetype FROM gs, student as st, gs_select
WHERE st.site = gs_select.siteid and grade >= lowgrade and grade <= highgrade and
st.studentid='$studentID' and gs.id = gs_select.gsid";
$grdtype=$db->get_var($sql);

$z = 0;
$data =array();

$gradeSymbols = getGradeSymbols($grdtype);

foreach ($subjects as $rec) {

	$sub = $rec['subject']; //$_GET['p2'];       // subject number
	$lvl = $rec['level']; //$_GET['p3'];      // level number

	$standards = getSubjectStandards ($sub, $lvl);
	$overall = getSubjectOverall ($sub, $lvl);
	$scores = getSutdentSubjectStandards($studentID, $sub, $lvl);
	$subInfo = getSubjectInfo ($sub, $lvl);

	$ch_name = $studentChanges[$sub][$lvl]['ch_name'];
	$ch_time = date($dateSettings, $studentChanges[$sub][$lvl]['ch_time']);

	$data[$sub]["subName"] = $subInfo[0]['name'];
	$data[$sub]["lvl"] = $rec['level'];
	$data[$sub]["levelName"] = $subInfo[0]['lname'];
	$data[$sub]["progress"] = $rec['progress'];
	$data[$sub]["studentText"] = "$studentName ($g1: $site)";
	$data[$sub]["changeText"] = "$ch_name ($ch_time)";
	$data[$sub]["totalStd"] = count($standards);
	$data[$sub]["std"] = array(); // Standard info
	$data[$sub]["standards"] = array(); // Student's score info

	for ($i = 0; $i < count($standards); $i++) {
		$std = $standards[$i]['std'];
		$link = $standards[$i]['link'];
		$value= getStdValue($std, $scores);
		$num = getToggleValue($value);
		$shortname = $standards[$i]['shortname'];
		$des = htmlentities($standards[$i]['description']);

		$wlist2[$z]['subject'] = $sub;
		$wlist2[$z]['level'] = $lvl;
		$wlist2[$z]['std'] = $std;
		$wlist2[$z]['num'] = $num;
		$wlist2[$z]['link'] = $link;
		$stdw = strlen($std) == 1 ? "0" . $std : $std;
		$wlist2[$z]['sort'] = $sub . "_" . $lvl . "_" . $stdw;
		$wlist2[$z]['shortname'] = $shortname;
		$z++;

		$type = isset($value)? "u" : "i";

		$des = str_replace("'", "\'", $des );

		$data[$sub]["std"][$i]["std"] = $std;
		$data[$sub]["std"][$i]["description"] = $des;
		$data[$sub]["std"][$i]["link"] = $link;
		$data[$sub]["standards"][$std]["name"] =
				"gr".$type."_".$studentID."_".$sub."_".$lvl."_".$std;
		$data[$sub]["standards"][$std]["value"] = getStdValue($std, $scores);
		$data[$sub]["standards"][$std]["selected"] = getEqGradeValue($grdtype, $value);
		$data[$sub]["standards"][$std]["symbol"] = getGradeSymbol($grdtype, $value);

	}

	/* Overall */
	$data[$sub]["totalOver"] = count($overall);
	$data[$sub]["over"] = array();
	$data[$sub]["overall"] = array();
	for ($i = 0; $i < count($overall); $i++) {
		$std = $overall[$i]['std'];
		$value = getStdValue($std, $scores);
		$des = str_replace("'", "\'", $overall[$i]['description']);

		$type = isset($value)? "u" : "i";

		$data[$sub]["over"][$i]["std"] = $std;
		$data[$sub]["over"][$i]["description"] = $des;
		$data[$sub]["over"][$i]["options"] = $gradeSymbols;

		$data[$sub]["overall"][$std]["name"] =
				"gr".$type."_".$studentID."_".$sub."_".$lvl."_".$std;
		$data[$sub]["overall"][$std]["value"] = getStdValue($std, $scores);
		$data[$sub]["overall"][$std]["selected"] = getEqGradeValue($grdtype, $value);
		$data[$sub]["overall"][$std]["symbol"] = getGradeSymbol($grdtype, $value);
	}
	/* Summary */
	$data[$sub]["summary"] = array();
	foreach($summary as $key => $val) {
		$value = getStdValue($key, $scores);
		$type = isset($value)? "u" : "i";
		$data[$sub]['summary'][$key]["name"] = "gr".$type."_".$studentID."_".$sub."_".$lvl."_".$key;
		$data[$sub]['summary'][$key]["value"] = $value;
	}
	/* Comment */
	$value = getStdValue('comment', $scores);
	$type = isset($value)? "u" : "i";
	$data[$sub]['comment'] = array(
		"comment_id" => "gr".$type."_".$studentID."_".$sub."_".$lvl."_comment",
		"value"  => $value,
		"esc_comment" => htmlspecialchars($value)
	);
}


$title = "Show Scores for Student";


// Weighted Standards for Student of users
//$wlist2 = getIndWeight ($studentID);
$wlist2 = array_sort($wlist2, 'num');
$wnum = min(30, count($wlist2));
$buckets = 5;
$t = $wlist2[0]['num'];
$b = $wlist2[$wnum-1]['num'];
$spread = ($t - $b)/$buckets;
//$wlist = $wlist2;
$wlist = array_slice($wlist2, 0, $wnum);
$wlist = array_sort($wlist, 'sort');
$weightedStd = array();
for ($i = 0; $i < $wnum; $i++) {
	$val = $wlist[$i]['num'];
	$sid = $wlist[$i]['subject'] . $wlist[$i]['level'] . "." . $wlist[$i]['std'];
	if($spread==0) {
		$tmpnum = 0;
	} else {
		$tmpnum = floor(($val - $b)/$spread);
	}
	$loc = $buckets - min($buckets-1, $tmpnum);
	$weightedStd[$i] = array();
	$weightedStd[$i]['link'] = $wlist[$i]['link'];
	$weightedStd[$i]['class'] = "t$loc";
	$weightedStd[$i]['span'] = $sid."-".$wlist[$i]['shortname'];

}

$smarty->assign('pageTitle',$title);
$smarty->assign('studentID',$studentID);
//$smarty->assign('reportLinks',$reportLinks);
//$smarty->assign('reportNames',$reportNames);
$smarty->assign('subjectMenu',$subjectMenu);

$smarty->assign('Display',$Display);
$smarty->assign('gradeSymbols',$gradeSymbols);

$smarty->assign('data',$data);

$smarty->assign('cellwidth',$cellwidth);
$smarty->assign('width',$width);
$smarty->assign('color1',$color1);

$smarty->assign('summary',$summary);
$smarty->assign('n_sum',count($summary));

$smarty->assign('weightedStd',$weightedStd);
$smarty->assign('studentName',$studentName);
$smarty->assign('subjectSelected',$subjectSelected);

//set flag to indicate that smarty template exists for this page
$isSmarty=true;
include ("template.inc");

$smarty->display('toggleStudent.tpl');

?>
