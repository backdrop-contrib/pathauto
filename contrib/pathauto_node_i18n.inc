<?php
// $Id$

/*
 * Implementation of hook_pathauto_node()
 */

function i18n_pathauto_node($op, $node=NULL) {
  switch ($op) {
    case 'placeholders':
      $placeholders = array();
      $placeholders[t('[lang]')] = t('Language code of the document');
      return $placeholders;
    case 'values':
      $results = array();
      $results[t('[lang]')] = i18n_node_get_lang($node->nid, i18n_get_lang());
      return $results;
    default:
      break;
  }
}

