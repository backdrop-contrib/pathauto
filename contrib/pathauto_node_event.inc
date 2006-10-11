<?php
// $Id$

/*
 * Implementation of hook_pathauto_node()
 */

function event_pathauto_node($op, $node=NULL) {
  switch ($op) {
    case 'placeholders':
      $placeholders = array();
      $placeholders[t('[eventyyyy]')] = t('The year the event starts.');
      $placeholders[t('[eventmm]')] = t('The two-digit month (01-12) the event starts.');
      $placeholders[t('[eventmon]')] = t('The three-letter month (jan-dec) the event starts.');
      $placeholders[t('[eventdd]')] = t('The two-digit day of the month (00-31) the event starts.');
      $placeholders[t('[eventday]')] = t('The three-letter day of the week (sun-sat) the event starts.');
      $placeholders[t('[eventweek]')] = t('The week number (1-52) of the year the event starts.');
      return $placeholders;
    case 'values':
      $results = array();
      $eventstart = $node->event_start;
      $results[t('[eventyyyy]')] = date('Y', $eventstart);
      $results[t('[eventmm]')] = date('m', $eventstart);
      $results[t('[eventmon]')] = pathauto_cleanstring(date('M', $eventstart));
      $results[t('[eventdd]')] = date('d', $eventstart);
      $results[t('[eventday]')] = pathauto_cleanstring(date('D', $eventstart));
      $results[t('[eventweek]')] = date('W', $eventstart);
      return $results;
    default:
      break;
  }
}

