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

      $tokens = token_info();
      foreach ($tokens['tokens']['user'] as $key => $info) {
        $settings['tokens']["[user:$key]"] = $info['description'];
      }

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

      $tokens = token_info();
      foreach ($tokens['tokens']['user'] as $key => $info) {
        $settings['tokens']["[user:$key]"] = $info['description'];
      }

      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk generate aliases for blogs that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing blog pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implements hook_pathauto().
 */
function tracker_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'tracker';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User-tracker paths');
      $settings['patterndescr'] = t('Pattern for user-tracker page paths');
      $settings['patterndefault'] = t('users/[user:name]/track');

      $tokens = token_info();
      foreach ($tokens['tokens']['user'] as $key => $info) {
        $settings['tokens']["[user:$key]"] = $info['description'];
      }

      $settings['supportsfeeds'] = 'feed';
      $settings['bulkname'] = t('Bulk generate aliases for user-tracker paths that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user-tracker pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Implements hook_pathauto().
 */
function contact_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'contact';
      $settings['token_type'] = 'user';
      $settings['groupheader'] = t('User personal contact form paths');
      $settings['patterndescr'] = t('Pattern for the user contact form paths');
      $settings['patterndefault'] = t('users/[user:name]/contact');

      $tokens = token_info();
      foreach ($tokens['tokens']['user'] as $key => $info) {
        $settings['tokens']["[user:$key]"] = $info['description'];
      }

      $settings['bulkname'] = t('Bulk generate aliases for user contact form paths that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing user contact form pages which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Bulk generate aliases for all users without aliases.
 */
function user_pathauto_bulkupdate() {
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON CONCAT('user/', CAST(uid AS CHAR)) = source WHERE uid > 0 AND source IS NULL";
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
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON CONCAT('blog/', CAST(uid AS CHAR)) = source WHERE uid > 0 AND source IS NULL";
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

/**
 * Bulk generate aliases for user trackers without aliases.
 */
function tracker_pathauto_bulkupdate() {
  // We do the double CONCAT because Pgsql8.1 doesn't support more than three arguments to CONCAT
  // Hopefully some day we can remove that.
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON CONCAT(CONCAT('user/', CAST(uid AS CHAR)), '/track') = source WHERE uid > 0 AND source IS NULL";
  $result = db_query_range($query, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  foreach ($result as $user) {
    $source = 'user/' . $user->uid . '/track';
    if (pathauto_create_alias('tracker', 'bulkupdate', $source, array('user' => $user))) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of user tracker pages completed, one alias generated.',
    'Bulk generation of user tracker pages completed, @count aliases generated.'));
}

/**
 * Bulk generate aliases for all users without aliases
 */
function contact_pathauto_bulkupdate() {
  $query = "SELECT uid, name, source, alias FROM {users} LEFT JOIN {url_alias} ON CONCAT(CONCAT('user/', CAST(uid AS CHAR)), '/contact') = source WHERE uid > 0 AND source IS NULL";
  $result = db_query_range($query, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  foreach ($result as $user) {
    $source = 'user/' . $user->uid . '/contact';
    if (pathauto_create_alias('contact', 'bulkupdate', $source, array('user' => $user))) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    'Bulk generation of contact pages completed, one alias generated.',
    'Bulk generation of contact pages completed, @count aliases generated.'));
}
