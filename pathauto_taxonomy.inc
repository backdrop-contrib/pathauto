<?php
// $Id$

/**
 * @file
 * Hook implementations for taxonomy module integration.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_pathauto().
 */
function taxonomy_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'taxonomy';
      $settings['token_type'] = 'term';
      $settings['groupheader'] = t('Taxonomy term paths');
      $settings['patterndescr'] = t('Default path pattern (applies to all vocabularies with blank patterns below)');
      $settings['patterndefault'] = t('category/[term:vocabulary]/[term:path]');
      $settings['supportsfeeds'] = '0/feed';
      $settings['bulkname'] = t('Bulk generate aliases for terms that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing terms which do not already have aliases.');

      $vocabularies = taxonomy_get_vocabularies();
      if (sizeof($vocabularies) > 0) {
        $settings['patternitems'] = array();
        $forum_vid = variable_get('forum_nav_vocabulary', '');
        foreach ($vocabularies as $vocab) {
          if ($vocab->vid != $forum_vid) {
            $vocabname = $vocab->name;
            $fieldlabel = t('Pattern for all %vocab-name paths', array('%vocab-name' => $vocabname));
            $settings['patternitems'][$vocab->vid] = $fieldlabel;
          }
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all categories without aliases.
 */
function taxonomy_pathauto_bulkupdate() {
  // Initialize db_or().
  $pattern_or = db_or();

  // From all vocabularies, only attempt to update those with patterns.
  foreach (taxonomy_get_vocabularies() as $vid => $info) {
    // TODO - If there's a default we shouldn't do this crazy where statement because all vocabularies get aliases.
    // TODO - Special casing to exclude the forum vid (and the images vid and...?).
    if (pathauto_pattern_load_by_entity('taxonomy', $vid)) {
      $pattern_or->condition('vid', $vid);
    }
  }

  // Set up the query object.
  $query = db_select('taxonomy_term_data', 'td');
  $concat = _pathauto_sql_concat("'taxonomy/term/'", 'td.tid');
  $query->leftJoin('url_alias', 'ua', "$concat = ua.source");
  $query->fields('td', array('tid', 'vid', 'name'));
  $query->fields('ua', array('source', 'alias'));
  $query->isNull('source');

  // Exclude the forums.
  if ($forum_vid = variable_get('forum_nav_vocabulary', '')) {
    $query->condition('vid', $forum_vid, '<>');
  }

  // Add $pattern_or condition to query.
  $query->condition($pattern_or);

  // Add range to query.
  $query->range(0, variable_get('pathauto_max_bulk_update', 50));

  // Execute query.
  $result = $query->execute();

  $count = 0;
  $placeholders = array();
  foreach ($result as $term) {
    $count += _taxonomy_pathauto_alias($term, 'bulkupdate');
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of terms completed, one alias generated.',
    'Bulk generation of terms completed, @count aliases generated.'));
}

/**
 * Create aliases for taxonomy objects.
 *
 * @param $category
 *   A taxonomy object.
 *
 * @todo Get rid of this function since we should only provide the $term object.
 */
function _taxonomy_pathauto_alias($term, $op) {
  $count = 0;
  $term = taxonomy_term_load($term->tid);
  $uri = entity_uri('taxonomy_term', $term);
  $forum_vid = variable_get('forum_nav_vocabulary', '');

  // If we're in a forum vocabulary, also create a forum container, forum, or forum topic alias.
  if (module_exists('forum') && $forum_vid == (int) $term->vid) {
    if (pathauto_create_alias('forum', $op, $uri['path'], array('term' => $term, 'vocabulary' => $vocabulary), $term->vid)) {
      $count++;
    }
  }
  else {
    if (pathauto_create_alias('taxonomy', $op, $uri['path'], array('term' => $term, 'vocabulary' => $vocabulary), $term->vid)) {
      $count++;
    }
  }
  return $count;
}

/**
 * Implement hook_pathauto() for forum module.
 */
function forum_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'forum';
      $settings['token_type'] = 'term';
      $settings['groupheader'] = t('Forum paths');
      $settings['patterndescr'] = t('Pattern for forums and forum containers');
      $settings['patterndefault'] = t('[term:vocabulary]/[term:path]');
      $settings['supportsfeeds'] = '0/feed';
      $settings['bulkname'] = t('Bulk generate aliases for forum paths that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing forums and forum containers which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all forums and forum containers without aliases.
 */
function forum_pathauto_bulkupdate() {
  // Set up the query object and limit it to the forum vocabulary.
  $query = db_select('taxonomy_term_data', 'td');
  $concat = _pathauto_sql_concat("'forum/'", 'td.tid');
  $query->leftJoin('url_alias', 'ua', "$concat = source");
  $query->condition('vid', variable_get('forum_nav_vocabulary', ''));
  $query->fields('td', array('tid', 'vid', 'name'));
  $query->fields('ua', array('source', 'alias'));
  $query->isNull('source');

  // Add range to query.
  $query->range(0, variable_get('pathauto_max_bulk_update', 50));

  // Execute query.
  $result = $query->execute();

  $count = 0;
  $placeholders = array();
  foreach ($result as $term) {
    $count += _taxonomy_pathauto_alias($term, 'bulkupdate');
  }

  drupal_set_message(format_plural($count,
    'Bulk update of forums and forum containers completed, one alias generated.',
    'Bulk update of forums and forum containers completed, @count aliases generated.'));
}
