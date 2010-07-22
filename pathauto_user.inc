<?php
// $Id$

/**
 * @file
 * Hook implementations for user module integration.
 *
 * @ingroup pathauto
 */

/**
 * Implements hook_pathauto().
 */
function user_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'user';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User paths');
      $settings['patterndescr'] = t('Pattern for user account page paths');
      $settings['patterndefault'] = t('users/[user:name]');
      $settings['bulkname'] = t('Bulk generate aliases for users that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user account pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implements hook_pathauto().
 */
function blog_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'blog';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('Blog paths');
      $settings['patterndescr'] = t('Pattern for blog page paths');
      $settings['patterndefault'] = t('blogs/[user:name]');
      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk generate aliases for blogs that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing blog pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Bulk generate aliases for all users without aliases.
 */
function user_pathauto_bulkupdate() {
  $concat = _pathauto_sql_concat("'user/'", 'uid');
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON $concat = source WHERE uid > 0 AND source IS NULL";
  $result = db_query_range($query, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  foreach ($result as $user) {
    $source = 'user/' . $user->uid;
    if (pathauto_create_alias('user', 'bulkupdate', $source, array('user' => $user))) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of users completed, one alias generated.',
    'Bulk generation of users completed, @count aliases generated.'));
}

/**
 * Bulk generate aliases for all blogs without aliases.
 */
function blog_pathauto_bulkupdate() {
  $concat = _pathauto_sql_concat("'blog/'", 'uid');
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON $concat = source WHERE uid > 0 AND source IS NULL";
  $result = db_query_range($query, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  foreach ($result as $user) {
    $source = 'blog/' . $user->uid;
    if (pathauto_create_alias('blog', 'bulkupdate', $source, array('user' => $user))) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of user blogs completed, one alias generated.',
    'Bulk generation of user blogs completed, @count aliases generated.'));
}
