<?php

/*
 * Implementation of hook_pathauto()
 */

function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['groupheader'] = t('Node path settings');
      $settings['patterndescr'] = t('Default path pattern (applies to all node types with blank patterns below)');
      $settings['patterndefault'] = t('[title]');
      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk update node paths');
      $settings['bulkdescr'] = 
        t('Generate aliases for all existing nodes which do not already have aliases.');
      $settings['placeholders'] = array(
        t('[title]') => t('The title of the node, with spaces and punctuation replaced by the separator.'),
        t('[nid]') => t('The id number of the node.'),
        t('[user]') => t('The name of the user who created the node.'),
        t('[type]') => t('The node type (e.g., "page", "story", etc.).'),
        t('[yyyy]') => t('The year the node was created.'),
        t('[mm]') => t('The two-digit month (01-12) the node was created.'),
        t('[mon]') => t('The three-letter month (jan-dec) the node was created.'),
        t('[dd]') => t('The two-digit day of the month (00-31) the node was created.'),
        t('[day]') => t('The three-letter day of the week (sun-sat) that the node was created.'),
        t('[hour]') => t('The two-digit hour (00-23) the node was created.'),
        t('[min]') => t('The two-digit minute (00-59) the node was created.'),
        t('[sec]') => t('The two-digit second (00-59) the node was created.'),
        t('[week]') => t('The week number (1-52) of the year the node was created.')
      );

      if (module_exist('taxonomy')) {
        $vocabularies = taxonomy_get_vocabularies();
        if (sizeof($vocabularies) > 0) {
          $settings['placeholders'][t('[vocab]')] =
            t("The vocabulary that the page's first category belongs to.");
          $settings['placeholders'][t('[cat]')] =
            t('The name of the lowest-weight category that the page belongs to.');
          $settings['placeholders'][t('[catpath]')] =
            t('As [cat], but including its supercategories.');
          $settings['placeholders'][t('[catalias]')] =
            t('The alias for the lowest-weight category that the page belongs to. '.
              'This is useful for long category names. You must first set up '.
              'aliases for your categories.');
        }
      }
      
      if (module_exist('book')) {
        $settings['placeholders'][t('[book]')] = 
          t('For book pages, the title of the top-level book.');
        $settings['placeholders'][t('[bookpath]')] = 
          t('For book pages, the full hierarchy from the top-level book.');
      }

      // Look for extensions from other modules
      $placeholders = module_invoke_all('pathauto_node', 'placeholders');
      $settings['placeholders'] = array_merge($settings['placeholders'], $placeholders);

      $nodetypes = node_get_types();
      foreach ($nodetypes as $ntype => $displaytype) {
        $fieldlabel = t('Pattern for all '.$displaytype.' paths');
        $settings['patternitems'][$ntype] = $fieldlabel;
      }
      return array2object($settings);
    default:
      break;
  }
}

/**
 * Implementation of hook_nodeapi().
 */
function pathauto_nodeapi(&$node, $op, $teaser, $page) {
  if (module_exist('path')) {
    switch ($op) {
      case 'insert':
      case 'update':
        $placeholders = node_get_placeholders($node);        
        $src = "node/$node->nid";
        $alias = pathauto_create_alias('node', $op, $placeholders, $src, $node->type);
        break;
      default:
        break;
    }
  }
}

/**
 * Implementation of hook_form_alter().
 */
function pathauto_form_alter($formid, &$form) {
  /*
  ** A helpful hint to the users with access to alias creation (pathauto
  ** will be invisible to other users). Inserted into the path module's
  ** fieldset in the node form
  */
  if ($form['#attributes']['id'] == 'node-form') {
    if (user_access('create url aliases')) {
      $output .= t('An alias will be automatically generated from the title '.
        'and other node attributes, in addition to any alias manually '.
        'provided above.');
      if (user_access('administer pathauto')) {
        $output .= t(' To control the format of the generated aliases, '.
          'see the <a href="admin/settings/pathauto">pathauto settings</a>.');
      }
    }
    if ($output) {
      $form['path']['pathauto'] = array('#type' => 'item', 
        '#description' => $output);
    }
  }
}

// Use the title and other node attributes to create an alias
function node_get_placeholders($node) {
  // Do the simple stuff first
  $placeholders[t('[title]')] = pathauto_cleanstring($node->title);
  $placeholders[t('[yyyy]')] = date('Y', $node->created);
  $placeholders[t('[mm]')] = date('m', $node->created);
  $placeholders[t('[mon]')] = pathauto_cleanstring(date('M', $node->created));
  $placeholders[t('[dd]')] = date('d', $node->created);
  $placeholders[t('[day]')] = pathauto_cleanstring(date('D', $node->created));
  $placeholders[t('[hour]')] = date('H', $node->created);
  $placeholders[t('[min]')] = date('i', $node->created);
  $placeholders[t('[sec]')] = date('s', $node->created);
  $placeholders[t('[week]')] = date('W', $node->created);
  $placeholders[t('[nid]')] = $node->nid;
  $placeholders[t('[type]')] = pathauto_cleanstring(node_get_name($node));
  
  // Then the slightly less simple stuff
  $result = db_query("SELECT name FROM {users} WHERE uid='%u'", $node->uid);
  $userinfo = db_fetch_object($result);
  $placeholders[t('[user]')] = pathauto_cleanstring($userinfo->name);
  
  // Book title
  if (module_exist('book') and $node->type == 'book') {
    $nodebook = book_load($node);
    $path = book_location($nodebook);
    $placeholders[t('[book]')] = pathauto_cleanstring($path[0]->title);
    $bookhierarchy = book_location($nodebook);
    $bookpath = '';
    foreach ($bookhierarchy as $bookelement) {
      if ($bookpath == '') {
        $bookpath = pathauto_cleanstring($bookelement->title);
      } else {
        $bookpath = $bookpath . '/' . pathauto_cleanstring($bookelement->title);
      }
    }
    $placeholders[t('[bookpath]')] = $bookpath;
  } else {
    $placeholders[t('[book]')] = '';
    $placeholders[t('[bookpath]')] = '';
  }

  // And now taxonomy, which is a bit more work
  if (module_exist('taxonomy') && is_array($node->taxonomy)) {
    // When editing a node, the taxonomy array might have one or more zero
    // term IDs. Ignore them...
    foreach ($node->taxonomy as $firsttermid) {
      if ($firsttermid) {
        break;
      }
    }
    $term = taxonomy_get_term($firsttermid);
    $placeholders[t('[cat]')] = pathauto_cleanstring($term->name);
    
    $vid = $term->vid;
    $vocabulary = taxonomy_get_vocabulary($vid);
    $placeholders[t('[vocab]')] = pathauto_cleanstring($vocabulary->name);
    
    $placeholders[t('[catalias]')] = drupal_get_path_alias('taxonomy/term/'.
      $node->taxonomy[0]);
    if (!strncasecmp($placeholders['[catalias]'], 'taxonomy', 8)) {
      $placeholders[t('[catalias]')] = $placeholders['[cat]'];
    }
    
    $catpath = '';
    $parents = taxonomy_get_parents_all($firsttermid);
    foreach ($parents as $parent) {
      $catpath = pathauto_cleanstring($parent->name).'/'.$catpath;
    }
    $placeholders[t('[catpath]')] = $catpath;
  } else {
    $placeholders[t('[cat]')] = '';
    $placeholders[t('[catpath]')] = '';
    $placeholders[t('[vocab]')] = '';
    $placeholders[t('[catalias]')] = '';
  }
  
  // Append any additional extensions
  $extplaceholders = module_invoke_all('pathauto_node', 'values', $node);

  $placeholders = array_merge($placeholders, $extplaceholders);
  
  return $placeholders;
}

// Generate aliases for all nodes without aliases
function node_pathauto_bulkupdate() {
  $query = 'SELECT nid,type,title,uid,created,src,dst FROM {node} '.
    "LEFT JOIN {url_alias} ON CONCAT('node/', nid) = src";
  $result = db_query($query);
  $node = db_fetch_object($result);

  $count = 0;
  $placeholders = array();
  while ($node) {
    if (module_exist('taxonomy')) {
      // Must populate the terms for the node here for the category
      // placeholders to work
      $node->taxonomy = array_keys(taxonomy_node_get_terms($node->nid));
    }
    $placeholders = node_get_placeholders($node);
    $src = "node/$node->nid";
    if ($alias = pathauto_create_alias('node', 'bulkupdate', $placeholders, 
                  $src, $node->type)) {
      $count++;
    }
    $node = db_fetch_object($result);
  }
  
  drupal_set_message(format_plural($count,
    "Bulk update of nodes completed, one alias generated.",
    "Bulk update of nodes completed, %count aliases generated."));
}

function node_pathauto_bulkupdate_indexes() {
  $query = "SELECT src,dst FROM {url_alias} WHERE src LIKE 'node/%'";
  $result = db_query($query);
  $alias = db_fetch_object($result);

  $count = 0;
  while ($alias) {
    $count += pathauto_create_index_alias($alias->dst, 'node');
    $alias = db_fetch_object($result);
  }
  return $count;
}

/**
 * Menu callback; presents a listing of all nodes matching an alias prefix
 */
function node_pathauto_page() {
  $prefix = '';
  $lastalpha = '';
  // Skip first two components, which will always be "pathauto/node"
  for ($i=2; arg($i); $i++) {
    $prefix .= arg($i) . '/';
    if (preg_match('/[a-z]/i', arg($i))) {
      $lastalpha = arg($i);
    }
  }
  
  if ($lastalpha) {
    $pagetitle = ucfirst($lastalpha);
  } else {
    $pagetitle = 'Listing';
  }

  $query = 'SELECT n.nid FROM {url_alias} a '.
    "INNER JOIN {node} n ON a.src = CONCAT('node/', n.nid)  ".
    "WHERE n.status = 1 AND a.dst LIKE '$prefix%' ".
    'ORDER BY n.sticky DESC, n.created DESC';
  $result = pager_query(db_rewrite_sql($query), variable_get('default_nodes_main', 10));

  $output = '';
  while ($node = db_fetch_object($result)) {
    $output .= node_view(node_load($node->nid), 1);
  }
  
  $output .= theme('pager', NULL, variable_get('default_nodes_main', 10));

  drupal_set_title(t($pagetitle));
  return $output;
}

?>