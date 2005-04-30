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
      $settings['bulkname'] = t('Bulk update node paths');
      $settings['bulkdescr'] = 
        t('Generate aliases for all existing nodes which do not already have aliases.');
      $settings['placeholders'] = array(
        t('[title]') => t('The title of the node, with spaces and punctuation.'),
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
            t('The name of the first category that the page belongs to.');
          $settings['placeholders'][t('[catpath]')] =
            t('As [cat], but including its supercategories.');
          $settings['placeholders'][t('[catalias]')] =
            t('The alias for the first category that the page belongs to. '.
              'This is useful for long category names. You must first set up '.
              'aliases for your categories.');
        }
      }
      
      if (module_exist('book')) {
        $settings['placeholders'][t('[book]')] = 
          t('For book pages, the title of the top-level book.');
      }
      
      $nodetypes = node_list();
      foreach ($nodetypes as $ntype) {
        if (module_exist('flexinode') and !strncasecmp($ntype, 'flexinode-', 10)) {
          $displaytype = flexinode_node_name($ntype);
        } else {
          $displaytype = $ntype;
        }
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
        $alias = pathauto_create_alias('node', $placeholders, $src, $node->type);
        break;
      case 'form pre':
        /*
        ** A helpful hint to the users with access to alias creation (pathauto
        ** will be invisible to other users). Because hook_nodeapi() is called
        ** in alphabetical order of module name, this will likely appear
        ** immediately after the path alias field.
        */
        $output = '';
        if (user_access('create url aliases')) {
          $output .= t('If the <b>Path alias</b> is left blank, an alias '.
            'will be automatically generated from the title and other '.
            'node attributes.');
          if (user_access('administer pathauto')) {
            $output .= t('To control the format of the generated aliases, '.
              'see the <a href="admin/settings/pathauto">pathauto settings</a>.');
          }
        }
        return $output;
      default:
        break;
    }
  }
}

// Use the title and other node attributes to create an alias
function node_get_placeholders($node) {
  // Do the simple stuff first
  $placeholders[t('[title]')] = $node->title;
  $placeholders[t('[yyyy]')] = date('Y', $node->created);
  $placeholders[t('[mm]')] = date('m', $node->created);
  $placeholders[t('[mon]')] = date('M', $node->created);
  $placeholders[t('[dd]')] = date('d', $node->created);
  $placeholders[t('[day]')] = date('D', $node->created);
  $placeholders[t('[hour]')] = date('H', $node->created);
  $placeholders[t('[min]')] = date('i', $node->created);
  $placeholders[t('[sec]')] = date('s', $node->created);
  $placeholders[t('[week]')] = date('W', $node->created);
  $placeholders[t('[nid]')] = $node->nid;
  
  // Then the slightly less simple stuff
  $result = db_query("SELECT name FROM {users} WHERE uid='%u'", $node->uid);
  $userinfo = db_fetch_object($result);
  $placeholders[t('[user]')] = $userinfo->name;
  
  if (module_exist('flexinode') and !strncasecmp($node->type, 'flexinode-', 10)) {
    $placeholders[t('[type]')] = flexinode_node_name($node->type);
  } else {
    $placeholders[t('[type]')] = $node->type;
  }

  // Book title
  if (module_exist('book') and $node->type == 'book') {
    $nodebook = book_load($node);
    $path = book_location($nodebook);
    $placeholders[t('[book]')] = $path[0]->title;
  } else {
    $placeholders[t('[book]')] = '';
  }

  // And now taxonomy, which is a bit more work
  if (module_exist('taxonomy')) {
    $firsttermid = $node->taxonomy[0];
    $term = taxonomy_get_term($firsttermid);
    $placeholders[t('[cat]')] = $term->name;
    
    $vid = $term->vid;
    $vocabulary = taxonomy_get_vocabulary($vid);
    $placeholders[t('[vocab]')] = $vocabulary->name;
    
    $placeholders[t('[catalias]')] = drupal_get_path_alias('taxonomy/term/'.
      $node->taxonomy[0]);
    if (!strncasecmp($placeholders['[catalias]'], 'taxonomy', 8)) {
      $placeholders[t('[catalias]')] = $placeholders['[cat]'];
    }
    
    $catpath = '';
    $parents = taxonomy_get_parents_all($firsttermid);
    foreach ($parents as $parent) {
      $catpath = $parent->name.'/'.$catpath;
    }
    $placeholders[t('[catpath]')] = $catpath;
  } else {
    $placeholders[t('[cat]')] = '';
    $placeholders[t('[catpath]')] = '';
    $placeholders[t('[vocab]')] = '';
    $placeholders[t('[catalias]')] = '';
  }
  return $placeholders;
}

// Generate aliases for all nodes without aliases
function node_pathauto_bulkupdate() {
  $query = 'SELECT nid,type,title,uid,created,src,dst FROM {node} '.
    "LEFT JOIN {url_alias} ON CONCAT('node/', nid) = src ".
    'WHERE dst is null';
  $result = db_query($query);
  $node = db_fetch_object($result);
  if (!$node) {
    drupal_set_message('No unaliased nodes to update.');
    return;
  }

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
    if ($alias = pathauto_create_alias('node', $placeholders, $src, $node->type)) {
      $count++;
    }
    $node = db_fetch_object($result);
  }
  
  drupal_set_message(format_plural($count,
    "Bulk update of nodes completed, one alias generated.",
    "Bulk update of nodes completed, %count aliases generated."));
}

?>