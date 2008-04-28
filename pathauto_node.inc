<?php
// $Id$

/*
 * Implementation of hook_pathauto()
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['groupheader'] = t('Node path settings');
      $settings['patterndescr'] = t('Default path pattern (applies to all node types with blank patterns below)');
      $settings['patterndefault'] = t('content/[title-raw]');
      $settings['bulkname'] = t('Bulk generate aliases for nodes that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing nodes which do not already have aliases.');

      $patterns = token_get_list('node');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['supportsfeeds'] = 'feed';

      $languages = module_exists('locale') ? locale_language_list() : array();
      foreach (node_get_types('names') as $node_type => $node_name) {
        $fieldlabel = t('Pattern for all @node_type paths', array('@node_type' => $node_name));
        $settings['patternitems'][$node_type] = $fieldlabel;
        foreach ($languages as $lang_code => $lang_name) {
          $fieldlabel = t('Pattern for all @node_type paths in @language', array('@node_type' => $node_name, '@language' => $lang_name));
          $settings['patternitems'][$node_type .'_'. $lang_code] = $fieldlabel;
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
    $pattern_types = array_keys(node_get_types('names')); 
  }
  else {
    // Check first for a node specific pattern...
    $languages = module_exists('locale') ? locale_language_list() : array();
    foreach (array_keys(node_get_types('names')) as $type) {
      if (trim(variable_get('pathauto_node_'. $type .'_pattern', ''))) {
        $pattern_types[$type] = $type;
        continue;
      }
      // ...then for a node-language pattern.
      foreach ($languages as $lang_code => $lang_name) {
        if (trim(variable_get('pathauto_node_'. $type .'_'. $lang_code .'_pattern', ''))) {
          $pattern_types[$type] = $type;
          continue 2;
        }
      }
    }
  }

  $count = 0;
  if (count($pattern_types)) {
    $query = "SELECT n.nid, n.vid, n.type, n.title, n.uid, n.created, alias.src, alias.dst, n.language FROM {node} n LEFT JOIN {url_alias} alias ON CONCAT('node/', n.nid) = alias.src WHERE alias.src IS NULL AND n.type IN (". db_placeholders($pattern_types, 'varchar') .')';
    $result = db_query_range($query, $pattern_types, 0, variable_get('pathauto_max_bulk_update', 50));

    $placeholders = array();
    while ($node_ref = db_fetch_object($result)) {
      $node = node_load($node_ref->nid, NULL, TRUE);
      $node->src = $node_ref->src;
      $node->dst = $node_ref->dst;
      if (module_exists('taxonomy')) {
        // Must populate the terms for the node here for the category
        // placeholders to work
        $node->taxonomy = array_keys(taxonomy_node_get_terms($node));
      }
      $placeholders = pathauto_get_placeholders('node', $node);
      $src = "node/$node->nid";
      if ($alias = pathauto_create_alias('node', 'bulkupdate', $placeholders, $src, $node->nid, $node->type, $node->language)) {
        $count++;
      }
    }
  }

  drupal_set_message(format_plural($count,
    "Bulk generation of nodes completed, one alias generated.",
    "Bulk generation of nodes completed, @count aliases generated."));
}
