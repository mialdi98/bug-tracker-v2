<?php

/**
 * @file
 * Maintains registration for api.
 */

require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');

define('BT_API_REG', 'bt_api_reg');

/**
 * Create new user.
 */
function reg_callback() {
  $values = reg_get_values();

  if (user_load_by_name($values['username']) || user_load_by_mail($values['email'])) {
    bt_api_response(409, BT_API_REG);
  }

  $account = new stdClass;
  $account->is_new = TRUE;
  $account->name = $values['username'];
  $account->pass = user_hash_password($values['password']);
  $account->mail = $values['email'];
  $account->init = $values['email'];
  $account->roles = $values['role'];
  $account->status = TRUE;
  $account->timezone = variable_get('date_default_timezone', '');
  user_save($account);

  bt_api_response(201, BT_API_REG);
}

/**
 * Get and process values.
 */
function reg_get_values() {
  // Get values.
  $values = [
    'username' => NULL,
    'password' => NULL,
    'email' => NULL,
    'role' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_REG);
  // Process username.
  $values['username'] = check_plain(trim($params['username'])) ?? bt_api_response(406, BT_API_REG);
  // Process password.
  $values['password'] = check_plain(trim($params['password'])) ?? bt_api_response(406, BT_API_REG);
  // Process email.
  $values['email'] = check_plain(trim($params['email'])) ?? t_api_response(406, BT_API_REG);
  // Process role.
  $role = check_plain(trim($params['role'])) ?? bt_api_response(406, BT_API_REG);
  $roles = bt_api_get_roles();
  $roles_keys = array_flip($roles);
  $role_id = isset($roles_keys[$role]) ? $roles_keys[$role] : bt_api_response(406, BT_API_REG);
  $values['role'] = [
    DRUPAL_AUTHENTICATED_RID => TRUE,
    $role_id => TRUE,
  ];

  return $values;
}
