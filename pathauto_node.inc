<?php
// $Id$

/**
 * @file
 * Hook implementations for node module integration.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_pathauto().
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['groupheader'] = t('Node paths');
      $settings['patterndescr'] = t('Default path pattern (applies to all node types with blank patterns below)');
      $settings['patterndefault'] = t('content/[node:title]');
      $settings['bulkname'] = t('Bulk generate aliases for nodes that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing nodes which do not already have aliases.');
      $settings['supportsfeeds'] = 'feed';

      if (module_exists('locale')) {
        $languages = array('' => t('Language neutral')) + locale_language_list('name');
      }
      else {
        $languages = array();
      }
      foreach (node_type_get_names() as $node_type => $node_name) {
        if (variable_get('language_content_type_' . $node_type, 0) && count($languages)) {
          $settings['patternitems'][$node_type] = t('Default path pattern for @node_type (applies to all @node_type node types with blank patterns below)', array('@node_type' => $node_name));
          foreach ($languages as $lang_code => $lang_name) {
            if (!empty($lang_code)) {
              $settings['patternitems'][$node_type . '_' . $lang_code] = t('Pattern for all @node_type paths in @language', array('@node_type' => $node_name, '@language' => $lang_name));
            }
            else {
              $settings['patternitems'][$node_type . '_' . $lang_code] = t('Pattern for all language neutral @node_type paths', array('@node_type' => $node_name));
            }
          }
        }
        else {
          $settings['patternitems'][$node_type] = t('Pattern for all @node_type paths', array('@node_type' => $node_name));
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all nodes without aliases.
 */
function node_pathauto_bulkupdate() {
  // From all node types, only attempt to update those with patterns
  $pattern_types = array();

  // If there's a default pattern we assume all types might be updated.
  if (trim(variable_get('pathauto_node_pattern', ''))) {
    $pattern_types = array_keys(node_type_get_types());
  }
  else {
    // Check first for a node specific pattern...
    $languages = array();
    if (module_exists('locale')) {
      $languages = array('' => t('Language neutral')) + locale_language_list('name');
    }
    foreach (array_keys(node_type_get_types()) as $type) {
      if (trim(variable_get('pathauto_node_' . $type . '_pattern', ''))) {
        $pattern_types[$type] = $type;
        continue;
      }
      // ...then for a node-language pattern.
      if (variable_get('language_content_type_' . $type, 0) && $languages) {
        foreach ($languages as $lang_code => $lang_name) {
          if (trim(variable_get('pathauto_node_' . $type . '_' . $lang_code . '_pattern', ''))) {
            $pattern_types[$type] = $type;
            continue 2;
          }
        }
      }
    }
  }

  $count = 0;
  if (count($pattern_types)) {
    // @TODO We could rewrite this section to use node_load_multiple
    $concat = _pathauto_sql_concat("'node/'", 'n.nid');
    $query = "SELECT n.nid, n.vid, n.type, n.title, n.uid, n.created, n.language, ua.source, ua.alias FROM {node} n LEFT JOIN {url_alias} ua ON $concat = ua.source WHERE ua.source IS NULL AND n.type IN (:pattern_types)";
    $result = db_query_range($query, 0, variable_get('pathauto_max_bulk_update', 50), array(':pattern_types' => $pattern_types));

    foreach ($result as $node_ref) {
      $node = node_load($node_ref->nid, NULL, TRUE);
      $node->source = $node_ref->source;
      $node->alias = $node_ref->alias;
      if (module_exists('taxonomy')) {
        // Must populate the terms for the node here for the category
        // placeholders to work
        // @TODO Find the Drupal 7 alternative for this if necessary
        //$node->taxonomy = array_keys(taxonomy_node_get_terms($node));
      }
      $uri = entity_uri('node', $node);
      if (pathauto_create_alias('node', 'bulkupdate', $uri['path'], array('node' => $node), $node->type, $node->language)) {
        $count++;
      }
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of nodes completed, one alias generated.',
    'Bulk generation of nodes completed, @count aliases generated.'));
}
