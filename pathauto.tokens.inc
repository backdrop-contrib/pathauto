<?php
// $Id$

/**
 * @file
 * Builds placeholder replacement tokens for pathauto.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_token_info().
 */
function pathauto_token_info() {
  $tokens = array();

  if (module_exists('taxonomy')) {
    //$tokens['node']['term-lowest'] = array(
    //  'name' => t('Lowest weighted term'),
    //  'description' => t('The taxonomy term associated with the node sorted by vocabulary weight, taxonomy term weight, then taxonomy term name.'),
    //  'type' => 'term',
    //);
  }

  return array(
    'tokens' => $tokens,
  );
}

/**
 * Implements hook_tokens().
 */
function pathauto_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $sanitize = !empty($options['sanitize']);

  // Node tokens.
  if ($type == 'node' && !empty($data['node'])) {
    $node = $data['node'];

    //foreach ($tokens as $name => $original) {
    //  switch ($name) {
    //  }
    //}

    // Chained token relationships.
    //if ($term_lowest_tokens = token_find_with_prefix($tokens, 'term-lowest')) {
    //  $tid = db_query_range('SELECT t.tid FROM {taxonomy_index} ti INNER JOIN {taxonomy_term_data} t ON ti.tid = t.tid INNER JOIN {taxonomy_vocabulary} v ON t.vid = v.vid WHERE ti.nid = :nid ORDER BY v.weight, t.weight, t.name', 0, 1, array(':nid' => $node->nid))->fetchField();
    //  if ($term = taxonomy_term_load($tid)) {
    //    $replacements += token_generate('term', $term_lowest_tokens, array('term' => $term), $options);
    //  }
    //}
  }

  return $replacements;
}
