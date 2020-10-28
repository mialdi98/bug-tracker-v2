<?php

/**
 * @file
 * Maintains auth for api.
 */

define('BT_API_AUTH', 'bt_api_auth');

/**
 * Login user.
 */
function auth_login_callback() {
  $values = auth_get_values();

  $uid = user_authenticate($values['username'], $values['password']);
  if ($uid === FALSE) {
    bt_api_response(403, BT_API_AUTH);
  }

  global $user;
  $user = user_load($uid);
  $credentials = ['name' => $values['username'], 'pass' => $values['password']];
  user_login_finalize($credentials);

  header('Access-Control-Expose-Headers: set-cookie');
  $cookie_key = array_keys($_COOKIE)[0];
  $cookie_value = $_COOKIE[$cookie_key];
  header("Set-Cookie: {$cookie_key}={$cookie_value};");
  bt_api_headers();
  bt_api_user_format($user);

  bt_api_response(202, BT_API_AUTH);
}

/**
 * Get and process values.
 */
function auth_get_values() {
  // Get values.
  $values = [
    'username' => NULL,
    'password' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_AUTH);
  // Process username.
  $values['username'] = check_plain(trim($params['username'])) ?? NULL;
  if ($values['username'] === NULL) {
    bt_api_response(406, BT_API_AUTH);
  }
  // Process password.
  $values['password'] = check_plain(trim($params['password'])) ?? NULL;
  if ($values['password'] === NULL) {
    bt_api_response(406, BT_API_AUTH);
  }

  return $values;
}

/**
 * Logout user.
 */
function auth_logout_callback() {
  module_load_include('pages.inc', 'user');
  user_logout_current_user();

  bt_api_response(200, BT_API_AUTH);
}

/**
 * Logout user.
 */
function auth_is_logged_in_callback() {
  module_load_include('pages.inc', 'user');
  if (user_is_logged_in()) {
    bt_api_response(200, BT_API_AUTH);
  }
  bt_api_response(401, BT_API_AUTH);
}
