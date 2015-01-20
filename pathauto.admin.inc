<?php

/**
 * @file
 * Admin page callbacks for the Pathauto module.
 *
 * @ingroup pathauto
 */

/**
 * Form builder; Configure the URL alias patterns.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function pathauto_patterns_form($form, $form_state) {
  $config = config('pathauto.settings');
  // Call the hook on all modules - an array of 'settings' objects is returned
  $all_settings = module_invoke_all('pathauto', 'settings');
  foreach ($all_settings as $settings) {
    $module = $settings->module;
    $patterndescr = $settings->patterndescr;
    $groupheader = $settings->groupheader;

    $form[$module] = array(
      '#type' => 'fieldset',
      '#title' => $groupheader,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    // Prompt for the default pattern for this module
    $variable = $module . '_pattern';
    $form[$module][$variable] = array(
      '#type' => 'textfield',
      '#title' => $patterndescr,
      '#default_value' => $config->get($variable),
      '#size' => 65,
      '#maxlength' => 1280,
      '#element_validate' => array('token_element_validate'),
      '#after_build' => array('token_element_validate'),
      '#token_types' => array($settings->token_type),
      '#min_tokens' => 1,
      '#parents' => array($variable),
    );

    // If the module supports a set of specialized patterns, set
    // them up here
    if (isset($settings->patternitems)) {
      foreach ($settings->patternitems as $itemname => $itemlabel) {
        $variable = $module . '_' . $itemname . '_pattern';
        $form[$module][$variable] = array(
          '#type' => 'textfield',
          '#title' => $itemlabel,
          '#default_value' => $config->get($variable),
          '#size' => 65,
          '#maxlength' => 1280,
          '#element_validate' => array('token_element_validate'),
          '#after_build' => array('token_element_validate'),
          '#token_types' => array($settings->token_type),
          '#min_tokens' => 1,
          '#parents' => array($variable),
        );
      }
    }

    // Display the user documentation of placeholders supported by
    // this module, as a description on the last pattern
    $form[$module]['token_help'] = array(
      '#title' => t('Replacement patterns'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form[$module]['token_help']['help'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array($settings->token_type),
    );
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}


function pathauto_patterns_form_submit($form, &$form_state) {
  $config = config('pathauto.settings');
  $values = $form_state['values'];

  $all_settings = module_invoke_all('pathauto', 'settings');
  foreach ($all_settings as $settings) {
    $module = $settings->module;
    // Save default pattern per module
    $variable = $module . '_pattern';
    $config->set($variable, $values[$variable]);

    // Save specialized patterns per module
    if (isset($settings->patternitems)) {
      foreach ($settings->patternitems as $itemname => $itemlabel) {
        $variable = $module . '_' . $itemname . '_pattern';
        $config->set($variable, $values[$variable]);
      }
    }
  }

  $config->save();
}

/**
 * Form builder; Configure the Pathauto settings.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function pathauto_settings_form($form) {
  module_load_include('inc', 'pathauto');
  $config = config('pathauto.settings');

  $form['verbose'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verbose'),
    '#default_value' => $config->get('verbose'),
    '#description' => t('Display alias changes (except during bulk updates).'),
  );

  $form['separator'] = array(
    '#type' => 'textfield',
    '#title' => t('Separator'),
    '#size' => 1,
    '#maxlength' => 1,
    '#default_value' => $config->get('separator'),
    '#description' => t('Character used to separate words in titles. This will replace any spaces and punctuation characters. Using a space or + character can cause unexpected results.'),
  );

  $form['case'] = array(
    '#type' => 'radios',
    '#title' => t('Character case'),
    '#default_value' => $config->get('case'),
    '#options' => array(
      PATHAUTO_CASE_LEAVE_ASIS => t('Leave case the same as source token values.'),
      PATHAUTO_CASE_LOWER => t('Change to lower case'),
    ),
  );

  $max_length = _pathauto_get_schema_alias_maxlength();

  $form['max_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum alias length'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => $config->get('max_length'),
    '#min_value' => 1,
    '#max_value' => $max_length,
    '#description' => t('Maximum length of aliases to generate. 100 is the recommended length. @max is the maximum possible length. See <a href="@pathauto-help">Pathauto help</a> for details.', array('@pathauto-help' => url('admin/help/pathauto'), '@max' => $max_length)),
    '#element_validate' => array('_pathauto_validate_numeric_element'),
  );
  $form['component_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum component length'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => $config->get('max_component_length'),
    '#min_value' => 1,
    '#max_value' => $max_length,
    '#description' => t('Maximum text length of any component in the alias (e.g., [title]). 100 is the recommended length. @max is the maximum possible length. See <a href="@pathauto-help">Pathauto help</a> for details.', array('@pathauto-help' => url('admin/help/pathauto'), '@max' => $max_length)),
    '#element_validate' => array('_pathauto_validate_numeric_element'),
  );


  $description = t('What should Pathauto do when updating an existing content item which already has an alias?');
  if (module_exists('redirect')) {
    $description .= ' ' . t('The <a href="!url">Redirect module settings</a> affect whether a redirect is created when an alias is deleted.', array('!url' => url('admin/config/search/redirect/settings')));
  }
  else {
    $description .= ' ' . t('Considering installing the <a href="!url">Redirect module</a> to get redirects when your aliases change.', array('!url' => 'http://drupal.org/project/redirect'));
  }
  $form['update_action'] = array(
    '#type' => 'radios',
    '#title' => t('Update action'),
    '#default_value' => $config->get('update_action'),
    '#options' => array(
      PATHAUTO_UPDATE_ACTION_NO_NEW => t('Do nothing. Leave the old alias intact.'),
      PATHAUTO_UPDATE_ACTION_LEAVE => t('Create a new alias. Leave the existing alias functioning.'),
      PATHAUTO_UPDATE_ACTION_DELETE => t('Create a new alias. Delete the old alias.'),
    ),
    '#description' => $description,
  );

  $form['transliterate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transliterate prior to creating alias'),
    '#default_value' => $config->get('transliterate') && module_exists('transliteration'),
    '#description' => t('When a pattern includes certain characters (such as those with accents) should Pathauto attempt to transliterate them into the US-ASCII alphabet? Transliteration is handled by the Transliteration module.'),
    '#access' => module_exists('transliteration'),
  );

  $form['reduce_ascii'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reduce strings to letters and numbers'),
    '#default_value' => $config->get('reduce_ascii'),
    '#description' => t('Filters the new alias to only letters and numbers found in the ASCII-96 set.'),
  );

  $form['ignore_words'] = array(
    '#type' => 'textarea',
    '#title' => t('Strings to Remove'),
    '#default_value' => $config->get('ignore_words'),
    '#description' => t('Words to strip out of the URL alias, separated by commas. Do not use this to remove punctuation.'),
    '#wysiwyg' => FALSE,
  );

  $form['punctuation'] = array(
    '#type' => 'fieldset',
    '#title' => t('Punctuation'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $details['default'] = PATHAUTO_PUNCTUATION_REMOVE;
    if ($details['value'] == $config->get('pathauto_separator')) {
      $details['default'] = PATHAUTO_PUNCTUATION_REPLACE;
    }
    $form['punctuation']['punctuation_' . $name] = array(
      '#type' => 'select',
      '#title' => $details['name'] . ' (<code>' . check_plain($details['value']) . '</code>)',
      '#default_value' => ($config->get('punctuation_' . $name)) ? $config->get('punctuation_' . $name): $details['default'],
      '#options' => array(
        PATHAUTO_PUNCTUATION_REMOVE => t('Remove'),
        PATHAUTO_PUNCTUATION_REPLACE => t('Replace by separator'),
        PATHAUTO_PUNCTUATION_DO_NOTHING => t('No action (do not replace)'),
      ),
    );
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function pathauto_settings_form_submit($form, &$form_state) {
  $config = config('pathauto.settings');
  $values = $form_state['values'];
  $config->set('verbose', $values['verbose']);
  $config->set('separator', $values['separator']);
  $config->set('case', $values['case']);
  $config->set('max_length', $values['max_length']);
  $config->set('component_length', $values['component_length']);
  $config->set('update_action', $values['update_action']);
  $config->set('transliterate', $values['transliterate']);
  $config->set('reduce_ascii', $values['reduce_ascii']);
  $config->set('ignore_words', $values['ignore_words']);

  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $config->set('punctuation_' . $name, $values['punctuation_' . $name]);
  }

  $config->save();
}

/**
 * Validate a form element that should have an numeric value.
 */
function _pathauto_validate_numeric_element($element, &$form_state) {
  $value = $element['#value'];

  if (!is_numeric($value)) {
    form_error($element, t('The field %name is not a valid number.', array('%name' => $element['#title'])));
  }
  elseif (isset($element['#max_value']) && $value > $element['#max_value']) {
    form_error($element, t('The field %name cannot be greater than @max.', array('%name' => $element['#title'], '@max' => $element['#max_value'])));
  }
  elseif (isset($element['#min_value']) && $value < $element['#min_value']) {
    form_error($element, t('The field %name cannot be less than @min.', array('%name' => $element['#title'], '@min' => $element['#min_value'])));
  }
}

/**
 * Validate pathauto_settings_form form submissions.
 */
function pathauto_settings_form_validate($form, &$form_state) {
  module_load_include('inc', 'pathauto');

  // Perform a basic check for HTML characters in the strings to remove field.
  if (strip_tags($form_state['values']['ignore_words']) != $form_state['values']['ignore_words']) {
    form_set_error('ignore_words', t('The <em>Strings to remove</em> field must not contain HTML. Make sure to disable any WYSIWYG editors for this field.'));
  }

  // Validate that the separator is not set to be removed per http://drupal.org/node/184119
  // This isn't really all that bad so warn, but still allow them to save the value.
  $separator = $form_state['values']['separator'];
  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    if ($details['value'] == $separator) {
      $action = $form_state['values']['punctuation_' . $name];
      if ($action == PATHAUTO_PUNCTUATION_REMOVE) {
        backdrop_set_message(t('You have configured the @name to be the separator and to be removed when encountered in strings. This can cause problems with your patterns and especially with the term:path token. You should probably set the action for @name to be "replace by separator".', array('@name' => $details['name'])), 'error');
      }
    }
  }
}

/**
 * Form contructor for path alias bulk update form.
 *
 * @see pathauto_bulk_update_form_submit()
 * @ingroup forms
 */
function pathauto_bulk_update_form() {
  $form['#update_callbacks'] = array();

  $form['update'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select the types of un-aliased paths for which to generate URL aliases'),
    '#options' => array(),
    '#default_value' => array(),
  );

  $pathauto_settings = module_invoke_all('pathauto', 'settings');
  foreach ($pathauto_settings as $settings) {
    if (!empty($settings->batch_update_callback)) {
      $form['#update_callbacks'][$settings->batch_update_callback] = $settings;
      $form['update']['#options'][$settings->batch_update_callback] = $settings->groupheader;
    }
  }

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

/**
 * Form submit handler for path alias bulk update form.
 *
 * @see pathauto_batch_update_form()
 * @see pathauto_bulk_update_batch_finished()
 */
function pathauto_bulk_update_form_submit($form, &$form_state) {
  $batch = array(
    'title' => t('Bulk updating URL aliases'),
    'operations' => array(
      array('pathauto_bulk_update_batch_start', array()),
    ),
    'finished' => 'pathauto_bulk_update_batch_finished',
    'file' => backdrop_get_path('module', 'pathauto') . '/pathauto.admin.inc',
  );

  foreach ($form_state['values']['update'] as $callback) {
    if (!empty($callback)) {
      $settings = $form['#update_callbacks'][$callback];
      if (!empty($settings->batch_file)) {
        $batch['operations'][] = array('pathauto_bulk_update_batch_process', array($callback, $settings));
      }
      else {
        $batch['operations'][] = array($callback, array());
      }
    }
  }

  batch_set($batch);
}

/**
 * Batch callback; count the current number of URL aliases for comparison later.
 */
function pathauto_bulk_update_batch_start(&$context) {
  $context['results']['count_before'] = db_select('url_alias')->countQuery()->execute()->fetchField();
}

/**
 * Common batch processing callback for all operations.
 *
 * Required to load our include the proper batch file.
 */
function pathauto_bulk_update_batch_process($callback, $settings, &$context) {
  if (!empty($settings->batch_file)) {
    require_once BACKDROP_ROOT . '/' . $settings->batch_file;
  }
  return $callback($context);
}

/**
 * Batch finished callback.
 */
function pathauto_bulk_update_batch_finished($success, $results, $operations) {
  if ($success) {
    // Count the current number of URL aliases after the batch is completed
    // and compare to the count before the batch started.
    $results['count_after'] = db_select('url_alias')->countQuery()->execute()->fetchField();
    $results['count_changed'] = max($results['count_after'] - $results['count_before'], 0);
    if ($results['count_changed']) {
      backdrop_set_message(format_plural($results['count_changed'], 'Generated 1 URL alias.', 'Generated @count URL aliases.'));
    }
    else {
      backdrop_set_message(t('No new URL aliases to generate.'));
    }
  }
  else {
    $error_operation = reset($operations);
    backdrop_set_message(t('An error occurred while processing @operation with arguments : @args', array('@operation' => $error_operation[0], '@args' => print_r($error_operation[0], TRUE))));
  }
}

/**
 * Menu callback; select certain alias types to delete.
 */
function pathauto_admin_delete() {
  /* TODO:
   1) all - DONE
   2) all node aliases - DONE
   4) all user aliases - DONE
   5) all taxonomy aliases - DONE
   6) by node type
   7) by taxonomy vocabulary
   8) no longer existing aliases (see http://drupal.org/node/128366 )
   9) where src like 'pattern' - DON'T DO
   10) where dst like 'pattern' - DON'T DO
  */

  $form['delete'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose aliases to delete'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  // First we do the "all" case
  $total_count = db_query('SELECT count(1) FROM {url_alias}')->fetchField();
  $form['delete']['all_aliases'] = array(
    '#type' => 'checkbox',
    '#title' => t('All aliases'),
    '#default_value' => FALSE,
    '#description' => t('Delete all aliases. Number of aliases which will be deleted: %count.', array('%count' => $total_count)),
  );

  // Next, iterate over an array of objects/alias types which can be deleted and provide checkboxes
  $objects = module_invoke_all('path_alias_types');
  foreach ($objects as $internal_name => $label) {
    $count = db_query("SELECT count(1) FROM {url_alias} WHERE source LIKE :src", array(':src' => "$internal_name%"))->fetchField();
    $form['delete'][$internal_name] = array(
      '#type' => 'checkbox',
      '#title' => $label, // This label is sent through t() in the hard coded function where it is defined
      '#default_value' => FALSE,
      '#description' => t('Delete aliases for all @label. Number of aliases which will be deleted: %count.', array('@label' => $label, '%count' => $count)),
    );
  }

  // Warn them and give a button that shows we mean business
  $form['warning'] = array('#value' => '<p>' . t('<strong>Note:</strong> there is no confirmation. Be sure of your action before clicking the "Delete aliases now!" button.<br />You may want to make a backup of the database and/or the url_alias table prior to using this feature.') . '</p>');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete aliases now!'),
  );

  return $form;
}

/**
 * Process pathauto_admin_delete form submissions.
 */
function pathauto_admin_delete_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if ($value) {
      if ($key === 'all_aliases') {
        db_delete('url_alias')
          ->execute();
        backdrop_set_message(t('All of your path aliases have been deleted.'));
      }
      $objects = module_invoke_all('path_alias_types');
      if (array_key_exists($key, $objects)) {
        db_delete('url_alias')
          ->condition('source', db_like($key) . '%', 'LIKE')
          ->execute();
        backdrop_set_message(t('All of your %type path aliases have been deleted.', array('%type' => $objects[$key])));
      }
    }
  }
  $form_state['redirect'] = 'admin/config/search/path/delete_bulk';
}
