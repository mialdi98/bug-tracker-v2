<?php

/**
 * @file
 * Page callbacks for Devel.
 */

/**
 * Page callback: Shows a menu item.
 */
function devel_menu_item() {
  $item = menu_get_item($_GET['path']);
  return kdevel_print_object($item);
}

/**
 * Page callback: Returns a list of all currently defined user functions.
 *
 * Returns a list of all currently defined user functions in the current request
 * lifecycle, with links their documentation.
 */
function devel_function_reference() {
  $functions = get_defined_functions();
  $version = devel_get_core_version(VERSION);
  $ufunctions = $functions['user'];
  sort($ufunctions);
  $api = variable_get('devel_api_url', 'api.drupal.org');
  foreach ($ufunctions as $function) {
    $links[] = l($function, "http://$api/api/$version/function/$function");
  }
  return theme('item_list', array('items' => $links));
}

/**
 * Page callback: Clears all caches, then redirects to the previous page.
 */
function devel_cache_clear() {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], 'devel-cache-clear')) {
    return MENU_ACCESS_DENIED;
  }

  drupal_flush_all_caches();

  drupal_set_message(t('Cache cleared.'));

  drupal_goto();
}

/**
 * Page callback: Called by the AJAX link in query log.
 */
function devel_querylog_explain($request_id, $qid) {
  if (!is_numeric($request_id)) {
    return MENU_ACCESS_DENIED;
  }

  $path = "temporary://devel_querylog/$request_id.txt";
  $path = file_stream_wrapper_uri_normalize($path);

  $output = t('No explain log found.');

  if (file_exists($path)) {
    $queries = json_decode(file_get_contents($path));

    if ($queries !== FALSE && isset($queries[$qid])) {
      $header = $rows = array();

      $query = $queries[$qid];
      $result = db_query('EXPLAIN ' . $query->query, (array)$query->args)->fetchAllAssoc('table');
      $i = 1;
      foreach ($result as $row) {
        $row = (array)$row;
        if ($i == 1) {
          $header = array_keys($row);
        }
        $rows[] = array_values($row);
        $i++;
      }

      $output = theme('table', array('header' => $header, 'rows' => $rows));
    }
  }

  // Print and return nothing thus avoiding page wrapper.
  print $output;
  $GLOBALS['devel_shutdown'] = FALSE;
}

/**
 * Page callback: Called by the AJAX link in query log.
 */
function devel_querylog_arguments($request_id, $qid) {
  if (!is_numeric($request_id)) {
    return MENU_ACCESS_DENIED;
  }

  $path = "temporary://devel_querylog/$request_id.txt";
  $path = file_stream_wrapper_uri_normalize($path);

  $output = t('No arguments log found.');

  if (file_exists($path)) {
    $queries = json_decode(file_get_contents($path));

    if ($queries !== FALSE && isset($queries[$qid])) {
      $query = $queries[$qid];
      $conn = Database::getConnection();
      $quoted = array();
      foreach ((array)$query->args as $key => $val) {
        $quoted[$key] = $conn->quote($val);
      }
      $output = strtr($query->query, $quoted);
    }
  }

  // Print and return nothing thus avoiding page wrapper.
  print $output;
  $GLOBALS['devel_shutdown'] = FALSE;
}

/**
 * Page callback: Clears the database, resetting the menu to factory defaults.
 */
function devel_menu_rebuild() {
  menu_rebuild();
  drupal_set_message(t('The menu router has been rebuilt.'));
  drupal_goto();
}

/**
 * Form constructor for reinstalling any module from a dropdown.
 *
 * @see devel_reinstall_submit()
 *
 * @ingroup forms
 */
function devel_reinstall($form, &$form_state) {
  $output = '';
  $modules = module_list();
  sort($modules);
  $options = drupal_map_assoc($modules);
  $form['list'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('Uninstall and then install the selected modules. <code>hook_uninstall()</code> and <code>hook_install()</code> will be executed and the schema version number will be set to the most recent update number. You may have to manually clear out any existing tables first if the module doesn\'t implement <code>hook_uninstall()</code>.'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#value' => t('Reinstall'),
    '#type' => 'submit',
  );
  return $form;
}

/**
 * Form submission callback for devel_reinstall().
 */
function devel_reinstall_submit($form, &$form_state) {
  // require_once './includes/install.inc';
  $modules = array_filter($form_state['values']['list']);
  module_disable($modules, FALSE);
  drupal_uninstall_modules($modules, FALSE);
  module_enable($modules, FALSE);
  drupal_set_message(t('Uninstalled and installed: %names.', array('%names' => implode(', ', $modules))));
}

/**
 * Page callback: Rebuilds the theme registry.
 */
function devel_theme_registry() {
  drupal_theme_initialize();
  $hooks = theme_get_registry();
  ksort($hooks);
  return kprint_r($hooks, TRUE);
}

/**
 * Page callback: Returns information from hook_entity_info().
 *
 * @param string $entity_type
 *   (optional) Limit results to the specified entity type.  The default is to
 *   return information about all entity types. The $entity_type argument is not
 *   currently used in the UI.
 *
 * @return
 *   The results from kprint_r().
 */
function devel_entity_info_page($entity_type = NULL) {
  $info = entity_get_info($entity_type);
  ksort($info);
  return kprint_r($info, TRUE);
}

/**
 * Page callback: Returns information about fields.
 */
function devel_field_info_page() {
  $info = field_info_fields();
  $output = kprint_r($info, TRUE, t('Fields'));
  $info = field_info_instances();
  $output .= kprint_r($info, TRUE, t('Instances'));
  $info = field_info_bundles();
  $output .= kprint_r($info, TRUE, t('Bundles'));
  $info = field_info_field_types();
  $output .= kprint_r($info, TRUE, t('Field types'));
  $info = field_info_formatter_types();
  $output .= kprint_r($info, TRUE, t('Formatter types'));
  $info = field_info_storage_types();
  $output .= kprint_r($info, TRUE, t('Storage types'));
  $info = field_info_widget_types();
  $output .= kprint_r($info, TRUE, t('Widget types'));
  return $output;
}

/**
 * Form constructor for displaying and editing variables.
 *
 * @see devel_variable_form_submit()
 *
 * @ingroup forms.
 */
function devel_variable_form() {
  $header = array(
    'name' => array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc'),
    'value' => array('data' => t('Value'), 'field' => 'value'),
    'length' => array('data' => t('Length'), 'field' => 'length'),
    'edit' => array('data' => t('Operations')),
  );
  // @todo: We could get variables out of $conf but that would include
  // hard-coded ones too. Ideally I would highlight overridden/hard-coded
  // variables.
  $query = db_select('variable', 'v')->extend('TableSort');
  $query->fields('v', array('name', 'value'));
  switch (db_driver()) {
    case 'mssql':
      $query->addExpression("LEN(v.value)", 'length');
      break;
    default:
      $query->addExpression("LENGTH(v.value)", 'length');
      break;
  }
  $result = $query
    ->orderByHeader($header)
    ->execute();

  foreach ($result as $row) {
    // $variables[$row->name] = '';
    $options[$row->name]['name'] = check_plain($row->name);
    if (merits_krumo($row->value)) {
      $value = krumo_ob(variable_get($row->name, NULL));
    }
    else {
      if (drupal_strlen($row->value) > 70) {
        $value = check_plain(drupal_substr($row->value, 0, 65)) . '...';
      }
      else {
        $value = check_plain($row->value);
      }
    }
    $options[$row->name]['value'] = $value;
    $options[$row->name]['length'] = $row->length;
    $options[$row->name]['edit'] = l(t('Edit'), "devel/variable/edit/$row->name");
  }
  $form['variables'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No variables.'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['#after_build'][] = 'devel_variable_form_after_build';

    // krumo($form);
  return $form;
}

/**
 * Form submission handler for devel_variable_form().
 */
function devel_variable_form_submit($form, &$form_state) {
  $deletes = array_filter($form_state['values']['variables']);
  array_walk($deletes, 'variable_del');
  if (count($deletes)) {
    drupal_set_message(format_plural(count($deletes), 'One variable deleted.', '@count variables deleted.'));
  }
}

/**
 * After build callback for devel_variable_form().
 *
 * Wrap each variable name in a <label> element.
 */
function devel_variable_form_after_build($form, &$form_state) {
  foreach ($form['variables']['#options'] as $variable => $element) {
    $form['variables']['#options'][$variable]['name'] = '<label for="' . $form['variables'][$variable]['#id'] . '">' . $element['name'] . '</label>';
  }
  return $form;
}

/**
 * Form constructor for editing a variable.
 *
 * @see devel_variable_edit_submit()
 *
 * @ingroup forms
 */
function devel_variable_edit($form, &$form_state, $name) {
  $value = variable_get($name, 'not found');
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $name
  );
  $form['value'] = array(
    '#type' => 'item',
    '#title' => t('Old value'),
    '#markup' => dpr($value, TRUE),
  );
  if (is_string($value) || is_numeric($value)) {
    $form['new'] = array(
      '#type' => 'textarea',
      '#title' => t('New value'),
      '#default_value' => $value
    );
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
  }
  else {
    $api = variable_get('devel_api_url', 'api.drupal.org');
    $form['new'] = array(
      '#type' => 'item',
      '#title' => t('New value'),
      '#markup' => t('Sorry, complex variable types may not be edited yet. Use the <em>Execute PHP</em> block and the <a href="@variable-set-doc">variable_set()</a> function.', array('@variable-set-doc' => "http://$api/api/HEAD/function/variable_set"))
    );
  }
  drupal_set_title($name);
  return $form;
}

/**
 * Form submission handler for devel_variable_edit().
 */
function devel_variable_edit_submit($form, &$form_state) {
  variable_set($form_state['values']['name'], $form_state['values']['new']);
  drupal_set_message(t('Saved new value for %name.', array('%name' => $form_state['values']['name'])));
   'devel/variable';
}

/**
 * Page callback: Displays the session.
 */
function devel_session() {
  global $user;
  $output = kprint_r($_SESSION, TRUE);
  $headers = array(t('Session name'), t('Session ID'));
  $output .= theme('table', array('headers' => $headers, 'rows' => array(array(session_name(), session_id()))));
  return $output;
}

/**
 * Page callback: Prints the loaded structure of the current node/user.
 */
function devel_load_object($type, $object, $name = NULL) {
  $name = isset($name) ? $name : $type;
  return kdevel_print_object($object, '$' . $name . '->');
}

/**
 * Page callback: Prints the render structure of the current object.
 *
 * @param string $type
 *   The type of entity that will be displayed.
 * @param object $object
 *   The entity to display.
 * @param string $name
 *   (optional) The label to use when displaying the entity.
 *
 * @return
 *   The results from kdevel_print_object().
 */
function devel_render_object($type, $object, $name = NULL) {
  $name = isset($name) ? $name : $type;
  $function = $type . '_view';

  switch ($function) {
    case 'comment_view':
      $node = node_load($object->nid);
      $build = $function($object, $node);
      break;

    default:
      $build = $function($object);
  }

  return kdevel_print_object($build, '$'. $name .'->');
}

/**
 * Page callback: Returns information from hook_elements().
 */
function devel_elements_page() {
  return kdevel_print_object(module_invoke_all('element_info'));
}

/**
 * Page callback: Returns the results from phpinfo().
 */
function devel_phpinfo() {
  print phpinfo();
  drupal_exit();
}
