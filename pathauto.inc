<?php
// $Id$

/**
 * @file
 * Miscellaneous functions for Pathauto.
 *
 * This also contains some constants giving human readable names to some numeric
 * settings; they're included here as they're only rarely used outside this file
 * anyway. Use _pathauto_include() if the constants need to be available.
 *
 * @ingroup pathauto
 */

/**
 * Case should be left as is in the generated path.
 */
define('PATHAUTO_CASE_LEAVE_ASIS', 0);

/**
 * Case should be lowercased in the generated path.
 */
define('PATHAUTO_CASE_LOWER', 1);

/**
 * "Do nothing. Leave the old alias intact."
 */
define('PATHAUTO_UPDATE_ACTION_NO_NEW', 0);

/**
 * "Create a new alias. Leave the existing alias functioning."
 */
define('PATHAUTO_UPDATE_ACTION_LEAVE', 1);

/**
 * "Create a new alias. Delete the old alias."
 */
define('PATHAUTO_UPDATE_ACTION_DELETE', 2);

/**
 * "Create a new alias. Redirect from old alias."
 *
 * This is only available when the Path Redirect module is.
 */
define('PATHAUTO_UPDATE_ACTION_REDIRECT', 3);

/**
 * Remove the punctuation from the alias.
 */
define('PATHAUTO_PUNCTUATION_REMOVE', 0);

/**
 * Replace the punctuation with the separator in the alias.
 */
define('PATHAUTO_PUNCTUATION_REPLACE', 1);

/**
 * Leave the punctuation as it is in the alias.
 */
define('PATHAUTO_PUNCTUATION_DO_NOTHING', 2);

/**
 * Check to see if there is already an alias pointing to a different item.
 *
 * @param $alias
 *   A string alias.
 * @param $source
 *   A string that is the internal path.
 * @param $language
 *   A string indicating the path's language.
 * @return
 *   TRUE if an alias exists, FALSE if not.
 */
function _pathauto_alias_exists($alias, $source, $language = LANGUAGE_NONE) {
  $alias_pid = db_query_range("SELECT pid FROM {url_alias} WHERE alias = :alias AND source <> :source AND language = :lang", 0, 1, array(':alias' => $alias, ':source' => $source, ':lang' => $language))->fetchField();
  if (module_exists('path_redirect') && function_exists('path_redirect_delete_multiple')) {
    // Delete from path_redirect the exact same alias to the same node.
    path_redirect_delete_multiple(NULL, array('source' => $alias, 'redirect' => $source));

    // If there still is this alias used in path_redirect, then create a different alias.
    $redirects = path_redirect_load_multiple(NULL, array('source' => $alias));
  }
  if ($alias_pid || !empty($redirects)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Returns old alias and pid if there is already an alias
 * pointing to a different item.
 *
 * @param $source
 *   A string that is the internal path.
 * @return
 *   An array with the keys "pid" and "old_alias" containing
 *   the "pid" and old "alias", respectively, of the old alias.
 */
function _pathauto_existing_alias_data($source) {
  $output = array(
    'pid' => '',
    'old_alias' => ''
  );
  $result = db_query("SELECT pid, alias FROM {url_alias} WHERE source = :source", array(':source' => $source));
  if ($data = $result->fetchObject()) {
    // The item is already aliased, check what to do...
    switch (variable_get('pathauto_update_action', PATHAUTO_UPDATE_ACTION_DELETE)) {
      // Replace old alias - remember the pid to update
      case PATHAUTO_UPDATE_ACTION_DELETE:
      case PATHAUTO_UPDATE_ACTION_REDIRECT:
        $output['pid'] = $data->pid;
      // Add new alias in addition to old one
      case PATHAUTO_UPDATE_ACTION_LEAVE:
        $output['old_alias'] = $data->alias;
        break;
      // Do nothing
      case PATHAUTO_UPDATE_ACTION_NO_NEW:
      default:
        break;
    }
  }
  return $output;
}

/**
 * Clean up a string value provided by a module.
 *
 * Resulting string contains only alphanumerics and separators.
 *
 * @param $string
 *   A string to clean.
 * @param $clean_slash
 *   Whether to clean slashes from the given string.
 * @return
 *   The cleaned string.
 */
function pathauto_cleanstring($string, $clean_slash = TRUE) {
  $output = $string;

  // Optionally transliterate (by running through the Transliteration module)
  if (variable_get('pathauto_transliterate', FALSE)) {
    if (module_exists('transliteration')) {
      $output = transliteration_get($output);
    }
    else {
      drupal_set_message(t('Pathauto could not transliterate the path, as the Transliteration module has been disabled.'), 'error');
    }
  }

  // Replace or drop punctuation based on user settings
  $separator = variable_get('pathauto_separator', '-');
  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $action = variable_get('pathauto_punctuation_' . $name, PATHAUTO_PUNCTUATION_REMOVE);
    if ($action != PATHAUTO_PUNCTUATION_DO_NOTHING) {
      // Slightly tricky inline if which either replaces with the separator or nothing
      $output = str_replace($details['value'], ($action ? $separator : ''), $output);
    }
  }

  // If something is already urlsafe then don't remove slashes
  if ($clean_slash) {
    $output = str_replace('/', '', $output);
  }

  // Reduce strings to letters and numbers
  if (variable_get('pathauto_reduce_ascii', FALSE)) {
    $pattern = '/[^a-zA-Z0-9\/]+/ ';
    $output = preg_replace($pattern, $separator, $output);
  }

  // Calculate and statically cache the ignored words regex expression.
  $ignore_words_regex = &drupal_static('pathauto_cleanstring:ignore_words_regex');
  if (!isset($ignore_words_regex)) {
    $ignore_words = variable_get('pathauto_ignore_words', PATHAUTO_IGNORE_WORDS);
    $ignore_words_regex = preg_replace(array('/^[,\s]+|[,\s]+$/', '/[,\s]+/'), array('', '\b|\b'), $ignore_words);
    if ($ignore_words_regex) {
      $ignore_words_regex = '\b' . $ignore_words_regex . '\b';
    }
  }

  // Get rid of words that are on the ignore list
  if ($ignore_words_regex) {
    if (function_exists('mb_eregi_replace')) {
      $words_removed = mb_eregi_replace($ignore_words_regex, '', $output);
    }
    else {
      $words_removed = preg_replace("/$ignore_words_regex/i", '', $output);
    }
    if (drupal_strlen(trim($words_removed)) > 0) {
      $output = $words_removed;
    }
  }

  // Always replace whitespace with the separator.
  $output = preg_replace('/\s+/', $separator, $output);

  // Trim duplicates and remove trailing and leading separators.
  $output = _pathauto_clean_separators($output, $separator);

  // Enforce the maximum component length
  $maxlength = min(variable_get('pathauto_max_component_length', 100), _pathauto_get_schema_alias_maxlength());
  $output = truncate_utf8($output, $maxlength, TRUE);

  return $output;
}

/**
 * Clean path separators from a given string.
 *
 * Trims duplicates and strips leading and trailing separators.
 *
 * @param $string
 *   The string to clean path separators from.
 * @param $separator
 *   The path separator to use when cleaning.
 * @return
 *   The cleaned version of the string.
 *
 * @see pathauto_cleanstring()
 * @see pathauto_clean_alias()
 */
function _pathauto_clean_separators($string, $separator = NULL) {
  $output = $string;

  // Clean duplicate or trailing separators.
  if (isset($separator) && strlen($separator)) {
    // Escape the separator.
    $seppattern = preg_quote($separator, '/');

    // Trim any leading or trailing separators.
    $output = preg_replace("/^$seppattern+|$seppattern+$/", '', $output);

    // Replace trailing separators around slashes.
    $output = preg_replace("/$seppattern+\/|\/$seppattern+/", "/", $output);

    // Replace multiple separators with a single one.
    $output = preg_replace("/$seppattern+/", $separator, $output);
  }

  // Optionally convert to lower case.
  if (variable_get('pathauto_case', PATHAUTO_CASE_LOWER)) {
    $output = drupal_strtolower($output);
  }

  return $output;
}

/**
 * Clean up an alias.
 *
 * This strips leading and trailing separators from the alias, and invokes
 * hook_pathauto_clean_alias() for other modules to have their chance.
 *
 * @param $alias
 *   A path alias to clean.
 * @param $clean_slash
 *   Whether to clean slashes from the alias..
 * @return
 *   The cleaned alias.
 */
function pathauto_clean_alias($alias) {
  $output = $alias;
  $separator = variable_get('pathauto_separator', '-');

  // Trim duplicates and remove trailing and leading separators.
  $output = _pathauto_clean_separators($output, $separator);

  // Give other modules a chance to clean this alias.
  foreach (module_implements('pathauto_clean_alias') as $module) {
    $function = $module .'_pathauto_clean_alias';
    $output = $function($output);
  }

  return $output;
}

/**
 * Apply patterns to create an alias.
 *
 * @param $module
 *   The name of your module (e.g., 'node').
 * @param $op
 *   Operation being performed on the content being aliased
 *   ('insert', 'update', 'return', or 'bulkupdate').
 * @param $source
 *   The "real" URI of the content to be aliased (e.g., "node/$node->nid").
 * @param $data
 *   An associative array of objects needed to apply the alias' pattern, keyed
 *   by their type.  Example: array('term' => $term, 'vocabulary' => $vocabulary).
 * @param $type
 *   For modules which provided pattern items in hook_pathauto(),
 *   the relevant identifier for the specific item to be aliased
 *   (e.g., $node->type).
 * @param $language
 *   A string specify the path's language.
 * @return
 *   The alias that was created.
 */
function pathauto_create_alias($module, $op, $source, $data, $type = NULL, $language = LANGUAGE_NONE) {
  if (($op != 'bulkupdate') and variable_get('pathauto_verbose', FALSE) && user_access('notify of path changes')) {
    $verbose = TRUE;
  }
  else {
    $verbose = FALSE;
  }

  // Retrieve and apply the pattern for this content type.
  $pattern = pathauto_pattern_load_by_entity($module, $type, $language);
  if (empty($pattern)) {
    // No pattern? Do nothing (otherwise we may blow away existing aliases...)
    return '';
  }

  if ($module == 'taxonomy' && isset($data['term'])) {
    // Get proper path for term.
    $term_path = 'taxonomy/term/' . $data['term']->tid;
    if ($term_path != $source) {
      // Quietly alias 'taxonomy/term/[tid]' with proper path for term.
      $update_data = _pathauto_existing_alias_data($source);
      _pathauto_set_alias($source, $term_path, $update_data['pid'], FALSE, $update_data['old_alias'], $language);
      // Set $source as proper path.
      $source = $term_path;
    }
  }

  // Special handling when updating an item which is already aliased.
  $pid = NULL;
  $old_alias = NULL;
  if ($op == 'update' or $op == 'bulkupdate') {
    if (variable_get('pathauto_update_action', PATHAUTO_UPDATE_ACTION_DELETE) == PATHAUTO_UPDATE_ACTION_NO_NEW) {
      // Do nothing
      return '';
    }
    $update_data = _pathauto_existing_alias_data($source);
    $pid = $update_data['pid'];
    $old_alias = $update_data['old_alias'];
  }

  // Replace any tokens in the pattern. Uses callback option to clean replacements. No sanitization.
  $alias = token_replace($pattern, $data, array('sanitize' => FALSE, 'clear' => TRUE, 'callback' => 'pathauto_clean_token_values', 'language' => (object) array('language' => $language)));

  // Two or more slashes should be collapsed into one
  $alias = preg_replace('/\/+/', '/', $alias);

  // Trim any leading or trailing slashes
  $alias = preg_replace('/^\/|\/+$/', '', $alias);

  // Clean the alias.
  $alias = pathauto_clean_alias($alias);

  // Shorten to a logical place based on the last separator.
  $separator = variable_get('pathauto_separator', '-');
  $maxlength = min(variable_get('pathauto_max_length', 100), _pathauto_get_schema_alias_maxlength());

  // Make sure we're not ending the alias in the middle of a word.
  $alias = truncate_utf8($alias, $maxlength, TRUE);

  // If the alias already exists, generate a new, hopefully unique, variant
  if (_pathauto_alias_exists($alias, $source, $language)) {
    $original_alias = $alias;
    for ($i = 0; _pathauto_alias_exists(drupal_substr($alias, 0, $maxlength - strlen($i)) . $separator . $i, $source, $language); $i++) {
    }
    // Make room for the sequence number
    $alias = drupal_substr($alias, 0, $maxlength - drupal_strlen($i));
    $alias = $alias . $separator . $i;
    // If verbose is on, alert the user why this happened
    if ($verbose) {
      drupal_set_message(t('The automatically generated alias %original_alias conflicted with an existing alias. Alias changed to %alias.',
        array('%original_alias' => $original_alias, '%alias' => $alias)));
    }
  }

  // Return the generated alias if requested.
  if ($op == 'return') {
    return $alias;
  }

  // If $pid is NULL, a new alias is created - otherwise, the existing
  // alias for the designated source is replaced.
  _pathauto_set_alias($source, $alias, $pid, $verbose, $old_alias, $language);

  // Also create a related feed alias if requested, and if supported
  // by the module
  if (drupal_strlen(variable_get('pathauto_' . $module . '_applytofeeds', ''))) {
    $feedappend = variable_get('pathauto_' . $module . '_applytofeeds', '');

    // For forums and taxonomies, the source doesn't always form the base of the rss feed (ie. image galleries)
    if (($module == 'taxonomy' || $module == 'forum') && isset($data['term'])) {
      $tid = $data['term']->tid;
      $update_data = _pathauto_existing_alias_data("taxonomy/term/$tid/$feedappend");
      _pathauto_set_alias("taxonomy/term/$tid/$feedappend", "$alias/feed", $update_data['pid'], $verbose, $update_data['old_alias'], $language);
    }
    else {
      $update_data = _pathauto_existing_alias_data("$source/$feedappend");
      _pathauto_set_alias("$source/$feedappend", "$alias/feed", $update_data['pid'], $verbose, $update_data['old_alias'], $language);
    }
  }

  return $alias;
}

/**
 * Verify if the given path is a valid menu callback.
 *
 * Taken from menu_execute_active_handler().
 *
 * @param $path
 *   A string containing a relative path.
 * @return
 *   TRUE if the path already exists.
 */
function _pathauto_path_is_callback($path) {
  $menu = menu_get_item($path);
  if (isset($menu['path']) && $menu['path'] == $path) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Private function for Pathauto to create an alias.
 *
 * @param $source
 *   The internal path.
 * @param $alias
 *   The visible externally used path.
 * @param $pid
 *   If the item is currently aliased, the pid for that item.
 * @param $verbose
 *   If the admin has enabled verbose, should be TRUE. Else FALSE or NULL.
 * @param $old_alias
 *   If the item is currently aliased, the existing alias for that item.
 * @param $language
 *   The path's language.
 */
function _pathauto_set_alias($source, $alias, $pid = NULL, $verbose = FALSE, $old_alias = NULL, $language = LANGUAGE_NONE) {
  // Alert users that an existing callback cannot be overridden automatically
  if (_pathauto_path_is_callback($alias)) {
    if ($verbose && user_access('notify of path changes')) {
      drupal_set_message(t('Ignoring alias %alias due to existing path conflict.', array('%alias' => $alias)));
    }
    return;
  }
  // Alert users if they are trying to create an alias that is the same as the internal path
  if ($source == $alias) {
    if ($verbose && user_access('notify of path changes')) {
      drupal_set_message(t('Ignoring alias %alias because it is the same as the internal path.', array('%alias' => $alias)));
    }
    return;
  }

  // Skip replacing the current alias with an identical alias
  if ($old_alias != $alias) {
    $path = array('source' => $source, 'alias' => $alias, 'pid' => $pid, 'language' => $language);
    path_save($path);

    if (variable_get('pathauto_update_action', PATHAUTO_UPDATE_ACTION_DELETE) == PATHAUTO_UPDATE_ACTION_REDIRECT
        && module_exists('path_redirect')
        && function_exists('path_redirect_save')) {
      if (!empty($old_alias)) {
        $redirect = array(
          'source' => $old_alias,
          'redirect' => $source,
        );
        path_redirect_save($redirect);
      }
    }
    if ($verbose && user_access('notify of path changes')) {
      if (!empty($redirect)) {
        drupal_set_message(t('Created new alias %alias for %source, replacing %old_alias. %old_alias now redirects to %alias.', array('%alias' => $alias, '%source' => $source, '%old_alias' => $old_alias)));
      }
      elseif ($pid) {
        drupal_set_message(t('Created new alias %alias for %source, replacing %old_alias.', array('%alias' => $alias, '%source' => $source, '%old_alias' => $old_alias)));
      }
      else {
        drupal_set_message(t('Created new alias %alias for %source.', array('%alias' => $alias, '%source' => $source)));
      }
    }
  }
}

/**
* Clean tokens so they are URL friendly.
*
* @param $replacements
*   An array of token replacements that need to be "cleaned" for use in the URL.
* @param $data
*   An array of objects used to generate the replacements.
* @param $options
*   An array of options used to generate the replacements.
*/
function pathauto_clean_token_values(&$replacements, $data = array(), $options = array()) {
 foreach ($replacements as $token => $value) {
   // If it's a "path" or "url friendly" token don't remove the "/" character
   if (drupal_substr($token, -5, 4) === 'path' || drupal_substr($token, -6, 5) === 'alias') {
     $replacements[$token] = pathauto_cleanstring($value, FALSE);
   }
   else {
     $replacements[$token] = pathauto_cleanstring($value);
   }
 }
}

/**
 * Return an array of arrays for punctuation values.
 *
 * Returns an array of arrays for punctuation values keyed by a name, including
 * the value and a textual description.
 * Can and should be expanded to include "all" non text punctuation values.
 *
 * @return
 *   An array of arrays for punctuation values keyed by a name, including the
 *   value and a textual description.
 */
function pathauto_punctuation_chars() {
  static $punctuation;

  if (!isset($punctuation)) {
    $punctuation = array();
    $punctuation['double_quotes']      = array('value' => '"', 'name' => t('Double quotes "'));
    $punctuation['quotes']             = array('value' => "'", 'name' => t("Single quotes (apostrophe) '"));
    $punctuation['backtick']           = array('value' => '`', 'name' => t('Back tick `'));
    $punctuation['comma']              = array('value' => ',', 'name' => t('Comma ,'));
    $punctuation['period']             = array('value' => '.', 'name' => t('Period .'));
    $punctuation['hyphen']             = array('value' => '-', 'name' => t('Hyphen -'));
    $punctuation['underscore']         = array('value' => '_', 'name' => t('Underscore _'));
    $punctuation['colon']              = array('value' => ':', 'name' => t('Colon :'));
    $punctuation['semicolon']          = array('value' => ';', 'name' => t('Semicolon ;'));
    $punctuation['pipe']               = array('value' => '|', 'name' => t('Pipe |'));
    $punctuation['left_curly']         = array('value' => '{', 'name' => t('Left curly bracket {'));
    $punctuation['left_square']        = array('value' => '[', 'name' => t('Left square bracket ['));
    $punctuation['right_curly']        = array('value' => '}', 'name' => t('Right curly bracket }'));
    $punctuation['right_square']       = array('value' => ']', 'name' => t('Right square bracket ]'));
    $punctuation['plus']               = array('value' => '+', 'name' => t('Plus +'));
    $punctuation['equal']              = array('value' => '=', 'name' => t('Equal ='));
    $punctuation['asterisk']           = array('value' => '*', 'name' => t('Asterisk *'));
    $punctuation['ampersand']          = array('value' => '&', 'name' => t('Ampersand &'));
    $punctuation['percent']            = array('value' => '%', 'name' => t('Percent %'));
    $punctuation['caret']              = array('value' => '^', 'name' => t('Caret ^'));
    $punctuation['dollar']             = array('value' => '$', 'name' => t('Dollar $'));
    $punctuation['hash']               = array('value' => '#', 'name' => t('Hash #'));
    $punctuation['at']                 = array('value' => '@', 'name' => t('At @'));
    $punctuation['exclamation']        = array('value' => '!', 'name' => t('Exclamation !'));
    $punctuation['tilde']              = array('value' => '~', 'name' => t('Tilde ~'));
    $punctuation['left_parenthesis']   = array('value' => '(', 'name' => t('Left parenthesis ('));
    $punctuation['right_parenthesis']  = array('value' => ')', 'name' => t('Right parenthesis )'));
    $punctuation['question_mark']      = array('value' => '?', 'name' => t('Question mark ?'));
    $punctuation['less_than']          = array('value' => '<', 'name' => t('Less than <'));
    $punctuation['greater_than']       = array('value' => '>', 'name' => t('Greater than >'));
    $punctuation['back_slash']         = array('value' => '\\', 'name' => t('Back slash \\'));
  }

  return $punctuation;
}

/**
 * Fetch the maximum length of the {url_alias}.alias field from the schema.
 *
 * @return
 *   An integer of the maximum url alias length allowed by the database.
 */
function _pathauto_get_schema_alias_maxlength() {
  $maxlength = &drupal_static(__FUNCTION__);
  if (!isset($maxlength)) {
    $schema = drupal_get_schema('url_alias');
    $maxlength = $schema['fields']['alias']['length'];
  }
  return $maxlength;
}
