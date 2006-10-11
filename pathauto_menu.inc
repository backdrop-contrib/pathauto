<?php
// $Id$

/**
 * @file
 * This is pathauto_menu.inc, an submodule for pathauto.module
 *
 * Allows you to generate url aliases that conform to the menu structure.
 */

/*
 * Implementation of hook_pathauto_node()
 */
function menu_pathauto_node($op, $node=NULL) {
  switch ($op) {
    case 'placeholders':
      $placeholders = array();
      $placeholders[t('[menu]')] =
        t('The name of the menu the node belongs to.');
      $placeholders[t('[menupath]')] =
        t('The menu path (as reflected in the breadcrumb), not including Home or [menu].');
      return $placeholders;
    case 'values':
      $results = pathauto_menu_get_placeholders('node/'.$node->nid);
      return $results;
    default:
      break;
  }
}

/**
 * Generate the menu placeholders.
 * 
 * @param $in
 *   Numeric input is treated a menu-id, strings as src-paths.
 * @return
 *   array() with [menu] and [menupath] placeholders
 */
function pathauto_menu_get_placeholders($in) {
  global $_menu;

  $trail = array();
  $placeholders = array();

  $mid = pathauto_menu_get_mid($in);

  while ($mid && $_menu['visible'][$mid]) {
    array_unshift($trail, pathauto_cleanstring($_menu['visible'][$mid]['title']));
    $mid = $_menu['visible'][$mid]['pid'];
  }

  if ( !empty($trail) ) {
    $placeholders[t('[menu]')] = array_shift($trail);
  }
  else {
    $placeholders[t('[menu]')] = '';
  }

  if ( !empty($trail) ) {
    $placeholders[t('[menupath]')] = implode('/', $trail);
  }
  else {
    $placeholders[t('[menupath]')] = '';
  }

  return $placeholders;
}

/**
 * Check if mid/path is present in the menu.
 *
 * @param $in
 *   Numeric input is treated a menu-id, strings as src-paths.
 * @return
 *   An existing mid, or 0 if none found.
 */
function pathauto_menu_get_mid($in) {
  global $_menu;

  if ( !is_numeric($in) ) {
    if ( isset($_menu['path index'][$in]) ) {
      $mid = $_menu['path index'][$in];
    }
    else {
      $mid = 0;
    }
  }
  else if ( !isset($_menu['visible'][$in]) ) {
    $mid = 0;
  }

  // temporary paths would break much of this module
  if ($mid < 0) $mid = 0;

  return $mid;
}

