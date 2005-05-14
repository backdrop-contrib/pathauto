<?php

/**
 * Implementation of hook_pathauto() for user aliases
 */
function user_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'user';
      $settings['groupheader'] = t('User path settings');
      $settings['patterndescr'] = t('Pattern for user account page paths');
      $settings['patterndefault'] = t('user/[user]');
      $settings['placeholders'] = array(
        t('[user]') => t('The name of the user.'),
        t('[uid]') => t('The id number of the user.')
      );
      $settings['bulkname'] = t('Bulk update user paths');
      $settings['bulkdescr'] = t('Generate aliases for all existing user account pages which do not already have aliases.');
      return array2object($settings);
    default:
      break;
  }
}

/**
 * Implementation of hook_pathauto() for blog aliases
 */
function blog_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'blog';
      $settings['groupheader'] = t('Blog path settings');
      $settings['patterndescr'] = t('Pattern for blog page paths');
      $settings['patterndefault'] = t('blog/[user]');
      $settings['placeholders'] = array(
        t('[user]') => t('The name of the user.'),
        t('[uid]') => t('The id number of the user.')
      );
      $settings['supportsfeeds'] = TRUE;
      $settings['bulkname'] = t('Bulk update blog paths');
      $settings['bulkdescr'] = t('Generate aliases for all existing blog pages which do not already have aliases.');
      return array2object($settings);
    default:
      break;
  }
}

/**
 * Implementation of hook_user() for both blog and user
 */
function pathauto_user($op, &$edit, &$user, $category=FALSE) {
  switch ($op) {
    case 'insert':
    case 'update':
      /*
      ** Use the username to automatically create an alias
      */
      $user = array2object($user);
      if ($user->name) {
        $placeholders = array();
        $placeholders[t('[user]')] = $user->name;
        $placeholders[t('[uid]')] = $user->uid;
        $src = 'user/'.$user->uid;
        $alias = pathauto_create_alias('user', $op, $placeholders, $src);
        if (module_exist('blog')) {
          $src = 'blog/'.$user->uid;
          $alias = pathauto_create_alias('blog', $op, $placeholders, $src);
        }
      }
      break;
    case 'delete':
      /*
      ** If the user is deleted, remove the path aliases
      **
      */
      $user = array2object($user);
      path_set_alias('user/'.$user->uid);
      if (module_exist('blog')) {
        path_set_alias('blog/'.$user->uid);
      }
      break;
    default:
      break;
  }
}

// Generate aliases for all users without aliases
function user_pathauto_bulkupdate() {
  $query = 'SELECT uid,name,src,dst FROM {users} '.
    "LEFT JOIN {url_alias} ON CONCAT('user/', uid) = src";
  $result = db_query($query);
  $user = db_fetch_object($result);

  $count = 0;
  $placeholders = array();
  while ($user) {
    $placeholders[t('[user]')] = $user->name;
    $placeholders[t('[uid]')] = $user->uid;
    $src = 'user/'.$user->uid;
    if ($alias = pathauto_create_alias('user', 'bulkupdate', $placeholders, $src)) {
      $count++;
    }
    $user = db_fetch_object($result);
  }
  
  drupal_set_message(format_plural($count,
    "Bulk update of users completed, one alias generated.",
    "Bulk update of users completed, %count aliases generated."));
}

// Generate aliases for all blogs without aliases
function blog_pathauto_bulkupdate() {
  $query = 'SELECT uid,name,src,dst FROM {users} '.
    "LEFT JOIN {url_alias} ON CONCAT('user/', uid) = src";
  $result = db_query($query);
  $user = db_fetch_object($result);

  $count = 0;
  $placeholders = array();
  while ($user) {
    $placeholders[t('[user]')] = $user->name;
    $placeholders[t('[uid]')] = $user->uid;
    $src = 'blog/'.$user->uid;
    if ($alias = pathauto_create_alias('blog', 'bulkupdate', $placeholders, $src)) {
      $count++;
    }
    $user = db_fetch_object($result);
  }

  drupal_set_message(format_plural($count,
    "Bulk update of user blogs completed, one alias generated.",
    "Bulk update of user blogs completed, %count aliases generated."));
}

?>