<?php

/**
 * @file
 * Maintains auth for api.
 */

define('BT_API_AUTH', 'bt_api_auth');
define('BT_API_AUTH_ONE_DAY_IN_SEC', 86400);
define('BT_API_JWT_LIB_ROUTE', DRUPAL_ROOT . '/sites/all/vendor/firebase/php-jwt/src');

include_once BT_API_JWT_LIB_ROUTE . '/BeforeValidException.php';
include_once BT_API_JWT_LIB_ROUTE . '/ExpiredException.php';
include_once BT_API_JWT_LIB_ROUTE . '/SignatureInvalidException.php';
include_once BT_API_JWT_LIB_ROUTE . '/JWT.php';

use Firebase\JWT\JWT;

/**
 * Login user.
 */
function auth_login_callback() {
  $values = auth_get_values();

  $uid = user_authenticate($values['username'], $values['password']);
  if ($uid === FALSE) {
    bt_api_response(401, BT_API_AUTH);
  }

  global $user;
  $user = user_load($uid);
  $credentials = ['name' => $values['username'], 'pass' => $values['password']];
  user_login_finalize($credentials);

  $auth = bt_api_user_format($user, FALSE);
  $data = ['jwt' => auth_jwt_generate($auth)];

  bt_api_response(202, BT_API_AUTH, TRUE, $data);
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
 * Generation of new token.
 *
 * Generated secret key - bin2hex(random_bytes(32))
 */
function auth_jwt_generate($auth) {
  global $base_url;
  $time = time();
  $iat = $time;
  $nbf = $time;
  $exp = $time + BT_API_AUTH_ONE_DAY_IN_SEC * 30;
  // Param iss - site url.
  // Param aud - site front end url (could be the same as iss).
  // Param iat - time when token was created.
  // Param nbf - time from when token can be used.
  // Param exp - time of token exporation.
  // Param data - data from which jwt will be generated.
  $payload = [
    'iss' => $base_url,
    'aud' => 'localhost', // $base_url
    'iat' => $iat,
    'nbf' => $nbf,
    'exp' => $exp,
    'data' => [
      'id' => $auth['id'],
      'username' => $auth['username'],
      'role' => $auth['role'],
    ],
  ];
  $jwt = JWT::encode($payload, variable_get('jwt_secret_key'));
  return $jwt;
}

/**
 * Working with token.
 */
function auth_get_user_by_token($token) {
  try {
    $decoded_token = JWT::decode($token, variable_get('jwt_secret_key'), ['HS256']);
  }
  catch (Throwable $t) {
    $type = get_class($t);
    if ($type === 'Firebase\JWT\ExpiredException'
    || $type === 'Firebase\JWT\SignatureInvalidException'
    || $type === 'Firebase\JWT\BeforeValidException') {
      bt_api_response(403, BT_API_AUTH);
    }
  }
  if (
    isset($decoded_token)
    && !empty($decoded_token)
    && isset($decoded_token->data->id)
    && !empty($decoded_token->data->id)
    && $user = user_load($decoded_token->data->id)
  ) {
    // Find user with neded uid.
    return $user;
  }
  return FALSE;
}
