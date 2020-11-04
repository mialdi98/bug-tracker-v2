<?php

/**
 * @file
 * Maintains project for api.
 */

define('BT_API_PROJECT', 'bt_api_project');

/**
 * Add new project.
 */
function project_add_callback() {
  $values = project_get_values($new = TRUE);
  global $user;
  $user = user_load($values['uid']);

  $node = new stdClass();
  $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] = $values['assignet_to'];
  $node->field_members[LANGUAGE_NONE][0]['target_id'] = reset($values['members']);
  $node->title = $values['title'];
  $node->language = LANGUAGE_NONE;
  $node->type = 'project';
  $node->uid = $user->uid;
  node_object_prepare($node);
  $node = node_submit($node);
  node_save($node);

  // Format to send data.
  $project = $values;
  unset($project['uid']);
  $project['id'] = $node->nid;
  $project['assignet_to'] = bt_api_user($project['assignet_to']);
  $project['members'] = bt_api_user(reset($project['members']));
  bt_api_headers();
  die(json_encode($project));
  bt_api_response(201, BT_API_PROJECT);
}

/**
 * Edit project.
 */
function project_edit_callback() {
  $values = project_get_values();
  global $user;
  $user = user_load($values['uid']);
  project_is_member_by_project_id($values['id']);

  $node = node_load($values['id']);
  if (empty($node)) {
    bt_api_response(406, BT_API_PROJECT);
  }
  $node->title = $values['title'] ?? bt_api_response(406, BT_API_PROJECT);
  $assignet_to_id = user_load_by_name($values['assignet_to'])->uid ?? bt_api_response(406, BT_API_PROJECT);
  $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] = $assignet_to_id;

  foreach ($values['members'] as $key => $member_username) {
    $member_id = user_load_by_name($member_username)->uid ?? bt_api_response(406, BT_API_PROJECT);
    $node->field_members[LANGUAGE_NONE][$key]['target_id'] = $member_id;
  }
  node_save($node);
  bt_api_headers();
  die(json_encode(['status' => 'Success']));
  bt_api_response(200, BT_API_PROJECT);
}

/**
 * Get and process values.
 */
function project_get_values($new = FALSE) {
  // Get values.
  if ($new === FALSE) {
    $values = [
      'id' => NULL,
      'title' => NULL,
      'assignet_to' => NULL,
      'members' => NULL,
      'uid' => NULL,
    ];
  }
  else {
    $values = [
      'title' => NULL,
      'assignet_to' => NULL,
      'members' => NULL,
      'uid' => NULL,
    ];
  }

  $params = bt_api_get_data($values, BT_API_PROJECT);
  if ($new === FALSE) {
    // Process id.
    $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_PROJECT);
  }
  // Process title.
  $values['title'] = check_plain(trim($params['title'])) ?? bt_api_response(406, BT_API_PROJECT);
  // Process assignet_to.
  $values['assignet_to'] = check_plain(trim($params['assignet_to'])) ?? bt_api_response(406, BT_API_PROJECT);
  // Process members.
  $values['members'] = $params['members'] ?? bt_api_response(406, BT_API_PROJECT);
  // Process user id.
  $values['uid'] = check_plain($params['uid']) ?? bt_api_response(406, BT_API_PROJECT);

  return $values;
}

/**
 * Delete project.
 */
function project_delete_callback() {
  // Get project id.
  $values = [
    'id' => NULL,
    'uid' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_PROJECT);
  $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_PROJECT);
  $values['uid'] = check_plain($params['uid']) ?? bt_api_response(406, BT_API_PROJECT);

  global $user;
  $user = user_load($values['uid']);
  project_is_member_by_project_id($values['id']);
  node_delete($values['id']);
  bt_api_headers();
  die(json_encode(['status' => 'Success']));
  bt_api_response(200, BT_API_PROJECT);
}

/**
 * Get project.
 */
function project_get_callback() {
  // Get project id.
  $values = [
    'id' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_PROJECT);
  $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_PROJECT);
  // Get nid of project where user is member.
  project_is_member_by_project_id($values['id']);
  $node_load = node_load_multiple([$values['id']], ['type' => 'project']);
  $node = reset($node_load);
  if (!empty($node)) {
    $project = [];
    $project['id'] = $node->nid ?? bt_api_response(406, BT_API_PROJECT);
    $project['title'] = $node->title ?? bt_api_response(406, BT_API_PROJECT);
    $project['assignet_to'] = bt_api_user($node->field_assignet_to[LANGUAGE_NONE][0]['target_id']);
    $members_ids = array_column($node->field_members[LANGUAGE_NONE], 'target_id') ?? bt_api_response(406, BT_API_PROJECT);
    foreach ($members_ids as $value) {
      $project['members'][] = bt_api_user($value);
    }
    bt_api_headers();
    die(json_encode($project));
  }
  bt_api_response(406, BT_API_PROJECT);
}

/**
 * Get all projects of user.
 */
function project_get_all_callback() {
  // Get nids of projects where user is member.
  $projects_ids = project_is_member_by_user_id();
  $nodes = node_load_multiple($projects_ids, ['type' => 'project']);
  if (!empty($nodes)) {
    $projects = [];
    foreach ($nodes as $node) {
      $project = [];
      $project['id'] = $node->nid ?? bt_api_response(406, BT_API_PROJECT);
      $project['title'] = $node->title ?? bt_api_response(406, BT_API_PROJECT);
      $project['assignet_to'] = bt_api_user($node->field_assignet_to[LANGUAGE_NONE][0]['target_id']);
      $members_ids = array_column($node->field_members[LANGUAGE_NONE], 'target_id') ?? bt_api_response(406, BT_API_PROJECT);
      foreach ($members_ids as $value) {
        $project['members'][] = bt_api_user($value);
      }
      $projects[] = $project;
    }
    bt_api_headers();
    die(json_encode($projects));
  }
  bt_api_response(406, BT_API_PROJECT);
}

/**
 * Get project of user by project id.
 */
function project_get_by_id($id) {
  global $user;
  $node_load = node_load_multiple([$id], ['type' => 'project']);
  $node = reset($node_load);
  if (!empty($node)) {
    $project = [];
    $project['id'] = $node->nid ?? bt_api_response(406, BT_API_PROJECT);
    $project['title'] = $node->title ?? bt_api_response(406, BT_API_PROJECT);
    $project['assignet_to'] = bt_api_user($node->field_assignet_to[LANGUAGE_NONE][0]['target_id']);
    $members_ids = array_column($node->field_members[LANGUAGE_NONE], 'target_id') ?? bt_api_response(406, BT_API_PROJECT);
    foreach ($members_ids as $value) {
      $project['members'][] = bt_api_user($value);
    }
    // Well formmed project array.
    return $project;
  }
  bt_api_response(406, BT_API_PROJECT);
}

/**
 * Check if user is member of project of task.
 */
function project_is_member_by_user_id() {
  global $user;
  $uid = bt_api_user_get();
  $user = user_load($uid);
  // Get nids of projects where user is member.
  $query = db_select('field_data_field_members', 'm');
  $query->fields('m', ['entity_id']);
  $query->condition('m.bundle', 'project');
  $query->condition('m.field_members_target_id', $user->uid);
  $result = $query->execute()->fetchAll();
  if (!empty($result)) {
    $projects_ids = array_column($result, 'entity_id');
    return $projects_ids;
  }
  bt_api_response(403, BT_API_PROJECT);
}

/**
 * Check if user is member of project of task.
 */
function project_is_member_by_project_id($id) {
  global $user;
  $uid = bt_api_user_get();
  $user = user_load($uid);
  // Get nids of projects where user is member.
  $query = db_select('field_data_field_members', 'm');
  $query->fields('m', ['entity_id']);
  $query->condition('m.bundle', 'project');
  $query->condition('m.entity_id', $id);
  $query->condition('m.field_members_target_id', $user->uid);
  $result = $query->execute()->fetchAll();
  if (!empty($result)) {
    return $result;
  }
  bt_api_response(403, BT_API_PROJECT);
}
