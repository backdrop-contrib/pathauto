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
    $tokens['term']['path'] = array(
      'name' => t('Taxonomy term path'),
      'description' => t('The name of the taxonomy term and all its parents in a path form. (%example)', array('%example' => 'parent-term/my-term')),
    );
    $tokens['term']['alias'] = array(
      'name' => t('Taxonomy term alias'),
      'description' => t('The URL alias of the taxonomy term.'),
    );
    //$tokens['node']['term-lowest'] = array(
    //  'name' => t('Lowest weighted term'),
    //  'description' => t('The taxonomy term associated with the node sorted by vocabulary weight, taxonomy term weight, then taxonomy term name.'),
    //  'type' => 'term',
    //);
  }

  if (module_exists('book')) {
    $tokens['node']['bookpathalias'] = array(
      'name' => t('Book Path Alias'),
      'description' => t('URL alias for the parent book.'),
    );
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

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'bookpathalias':
          if (module_exists('book') && !empty($node->book['plid']) && $parent = book_link_load($node->book['plid'])) {
            $replacements[$original] = drupal_get_path_alias($parent['href']);
          }
          break;
      }
    }

    // Chained token relationships.
    //if ($term_lowest_tokens = token_find_with_prefix($tokens, 'term-lowest')) {
    //  $tid = db_query_range('SELECT t.tid FROM {taxonomy_index} ti INNER JOIN {taxonomy_term_data} t ON ti.tid = t.tid INNER JOIN {taxonomy_vocabulary} v ON t.vid = v.vid WHERE ti.nid = :nid ORDER BY v.weight, t.weight, t.name', 0, 1, array(':nid' => $node->nid))->fetchField();
    //  if ($term = taxonomy_term_load($tid)) {
    //    $replacements += token_generate('term', $term_lowest_tokens, array('term' => $term), $options);
    //  }
    //}
  }

  if ($type == 'term' && !empty($data['term'])) {
    $term = $data['term'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'path':
          module_load_include('inc', 'pathauto');
          $parents = taxonomy_get_parents_all($term->tid);
          $catpath = array();
          foreach ($parents as $parent) {
            $term_name = $sanitize ? check_plain($parent->name) : $parent->name;
            array_unshift($catpath, pathauto_cleanstring($term_name));
          }
          if ($catpath) {
            $replacements[$original] = implode('/', $catpath);
          }
          break;
        case 'alias':
          $replacements[$original] = drupal_get_path_alias('taxonomy/term/' . $term->tid);
          break;
      }
    }
  }

  return $replacements;
}
