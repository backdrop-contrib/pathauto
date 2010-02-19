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
    $tokens['term']['catpath'] = array(
      'name' => t("Node's Term Path"),
      'description' => t('As [cat], but including its supercategories separated by /.'),
    );
    $tokens['term']['catalias'] = array(
      'name' => t("Node's Term Alias"),
      'description' => t('URL alias for the term.'),
    );

    $tokens['node']['termpath'] = array(
      'name' => t('Term Path'),
      'description' => t('As [term], but including its supercategories separated by /.'),
    );
    $tokens['node']['termalias'] = array(
      'name' => t('Term Alias'),
      'description' => t('URL alias for the term.'),
    );

    if (module_exists('book')) {
      $tokens['node']['bookpathalias'] = array(
        'name' => t('Book Path Alias'),
        'description' => t('URL alias for the parent book.'),
      );
    }
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
  if (module_exists('taxonomy')) {
    if ($type == 'term' || $type == 'node') {
      _pathauto_include();
      $sanitize = !empty($options['sanitize']);
      $separator = variable_get('pathauto_separator', '-');
      $catpath = $alias = '';

      if (($type == 'node' && !empty($data['node'])) || ($type == 'term' && !empty($data['term']))) {
        if ($type == 'node') {
          $node = $data['node'];
          // Get taxonomy related data.
          $vid = db_query_range("SELECT t.vid FROM {taxonomy_index} i INNER JOIN {taxonomy_term_data} t ON i.tid = t.tid INNER JOIN {taxonomy_vocabulary} v ON t.vid = v.vid WHERE i.nid = :nid ORDER BY v.weight, t.weight, t.name", 0, 1, array(':nid' => $node->nid))->fetchColumn();
          $term = db_query_range("SELECT t.tid, t.name FROM {taxonomy_term_data} t INNER JOIN {taxonomy_index} i ON i.tid = t.tid WHERE t.vid = :vid AND i.nid = :nid ORDER BY t.weight", 0, 1, array(':vid' => $vid, ':nid' => $node->nid))->fetchObject();
          $term->vid = $vid;
        }
        elseif ($type == 'term') {
          $term = $data['term'];
        }

        if (isset($term->tid)) {
          $parents = taxonomy_get_parents_all($term->tid);
          array_shift($parents);
          foreach ($parents as $parent) {
            $catpath = '/' . pathauto_cleanstring(check_plain($parent->name)) . $catpath;
          }

          $catpath = $catpath . '/' . pathauto_cleanstring(check_plain($term->name));
          $alias = drupal_get_path_alias('taxonomy/term/' . $term->tid);
          if (!strncasecmp($alias, 'taxonomy', 8)) {
            $alias = check_plain($term->name);
          }
        }

        foreach ($tokens as $name => $original) {
          switch ($name) {
            case 'bookpathalias':
              $replacements[$original] = '';
              if (module_exists('book') && !empty($node->book['plid']) && $parent = book_link_load($node->book['plid'])) {
                $replacements[$original] = drupal_get_path_alias($parent['href']);
              }
              break;

            case 'termpath':
            case 'catpath':
              $replacements[$original] = $catpath;
              break;

            case 'termalias':
            case 'catalias':
              $replacements[$original] = $alias;
              break;
          }
        }
      }
    }
  }
  return $replacements;
}
