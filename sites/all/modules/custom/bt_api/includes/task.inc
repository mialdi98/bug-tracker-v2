<?php

/**
 * @file
 * Maintains task for api.
 */

define('BT_API_TASK', 'bt_api_task');

/**
 * Add new task.
 */
function task_add_callback() {
  $values = task_get_values($new = TRUE);
  task_is_member_by_project_id($values['project'], $new = TRUE);

  global $user;
  $node = new stdClass();
  $status = taxonomy_get_term_by_name($values['status'], 'status');
  $status_id = reset($status)->tid ?? bt_api_response(406, BT_API_TASK);
  $node->field_status[LANGUAGE_NONE][0]['tid'] = $status_id;
  $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] = $values['assignet_to'];
  $node->field_project[LANGUAGE_NONE][0]['target_id'] = $values['project'];
  $node->field_link[LANGUAGE_NONE][0]['value'] = $values['link'] ?? '';
  $node->field_description[LANGUAGE_NONE][0]['value'] = $values['description'] ?? '';
  $node->title = $values['title'];
  $node->language = LANGUAGE_NONE;
  $node->type = 'task';
  $node->uid = $user->uid;
  node_object_prepare($node);
  $node = node_submit($node);
  node_save($node);

  // Format to send data.
  $task = $values;
  unset($task['uid']);
  $task['id'] = $node->nid;
  $task['assignet_to'] = bt_api_user($task['assignet_to']);
  bt_api_headers();
  die(json_encode($task));
  bt_api_response(201, BT_API_TASK);
}

/**
 * Edit task.
 */
function task_edit_callback() {
  $values = task_get_values();
  task_is_member_by_task_id($values['id']);

  $node = node_load($values['id']);
  if (empty($node)) {
    bt_api_response(406, BT_API_TASK);
  }
  $status = taxonomy_get_term_by_name($values['status'], 'status');
  $status_id = reset($status)->tid ?? bt_api_response(406, BT_API_TASK);
  $node->field_status[LANGUAGE_NONE][0]['tid'] = $status_id;
  $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] = $values['assignet_to'];
  $node->field_project[LANGUAGE_NONE][0]['target_id'] = $values['project'];
  $node->field_link[LANGUAGE_NONE][0]['value'] = $values['link'] ?? '';
  $node->field_description[LANGUAGE_NONE][0]['value'] = $values['description'] ?? '';
  $node->title = $values['title'];
  node_save($node);

  // Format to send data.
  $task = $values;
  unset($task['uid']);
  $task['id'] = $node->nid;
  $task['assignet_to'] = bt_api_user($task['assignet_to']);
  bt_api_headers();
  die(json_encode($task));
  bt_api_response(200, BT_API_TASK);
}

/**
 * Get and process values.
 */
function task_get_values($new = FALSE) {
  // Get values.
  if ($new === FALSE) {
    $values = [
      'id' => NULL,
      'title' => NULL,
      'link' => NULL,
      'status' => NULL,
      'assignet_to' => NULL,
      'description' => NULL,
      'project' => NULL,
    ];
  }
  else {
    $values = [
      'title' => NULL,
      'link' => NULL,
      'status' => NULL,
      'assignet_to' => NULL,
      'description' => NULL,
      'project' => NULL,
    ];
  }

  $params = bt_api_get_data($values, BT_API_TASK);
  if ($new === FALSE) {
    // Process id.
    $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_TASK);
  }
  // Process title.
  $values['title'] = check_plain(trim($params['title'])) ?? bt_api_response(406, BT_API_TASK);
  // Process link.
  $values['link'] = check_plain(trim($params['link'])) ?? bt_api_response(406, BT_API_TASK);
  // Process status.
  $values['status'] = check_plain(trim($params['status'])) ?? bt_api_response(406, BT_API_TASK);
  // Process description.
  $values['description'] = check_plain(trim($params['description'])) ?? bt_api_response(406, BT_API_TASK);
  // Process assignet_to.
  $values['assignet_to'] = check_plain(trim($params['assignet_to'])) ?? bt_api_response(406, BT_API_TASK);
  // Process project.
  $values['project'] = check_plain(trim($params['project'])) ?? bt_api_response(406, BT_API_TASK);

  return $values;
}

/**
 * Delete task.
 */
function task_delete_callback() {
  // Get task id.
  $values = [
    'id' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_TASK);
  $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_TASK);
  task_is_member_by_task_id($values['id']);
  node_delete($values['id']);
  bt_api_headers();
  die(json_encode(['status' => 'Success']));
  bt_api_response(200, BT_API_TASK);
}

/**
 * Get all tasks of user.
 */
function task_get_all_callback() {
  // Get project id.
  $values = [
    'id' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_TASK);
  // Process project id.
  $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_TASK);
  $tasks_ids = task_is_member_by_project_id($values['id']);
  $nodes = node_load_multiple($tasks_ids, ['type' => 'task']);
  if (!empty($nodes)) {
    $tasks = [];
    foreach ($nodes as $node) {
      $task = [];
      $task['id'] = $node->nid ?? bt_api_response(406, BT_API_TASK);
      $task['title'] = $node->title ?? bt_api_response(406, BT_API_TASK);
      $task['link'] = $node->field_link[LANGUAGE_NONE][0]['value'] ?? '';
      $task['description'] = $node->field_description[LANGUAGE_NONE][0]['value'] ?? '';
      $status_id = $node->field_status[LANGUAGE_NONE][0]['tid'] ?? bt_api_response(406, BT_API_TASK);
      $task['status'] = taxonomy_term_load($status_id)->name ?? bt_api_response(406, BT_API_TASK);
      $assignet_to_id = $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] ?? NULL;
      $task['assignet_to'] = bt_api_user($assignet_to_id) ?? bt_api_response(406, BT_API_TASK);
      $project_id = $node->field_project[LANGUAGE_NONE][0]['target_id'] ?? NULL;
      // Also get project.
      $task['project'] = project_get_by_id($project_id) ?? bt_api_response(406, BT_API_TASK);
      $tasks[] = $task;
    }
    bt_api_headers();
    die(json_encode($tasks));
  }
  bt_api_response(406, BT_API_TASK);
}

/**
 * Get specific task.
 */
function task_get_callback() {
  // Get task id.
  $values = [
    'id' => NULL,
  ];
  $params = bt_api_get_data($values, BT_API_TASK);
  // Process task id.
  $values['id'] = check_plain($params['id']) ?? bt_api_response(406, BT_API_TASK);
  task_is_member_by_task_id($values['id']);
  $node_load = node_load_multiple([$values['id']], ['type' => 'task']);
  $node = reset($node_load);
  if (!empty($node)) {
    $task = [];
    $task['id'] = $node->nid ?? bt_api_response(406, BT_API_TASK);
    $task['title'] = $node->title ?? bt_api_response(406, BT_API_TASK);
    $task['link'] = $node->field_link[LANGUAGE_NONE][0]['value'] ?? '';
    $task['description'] = $node->field_description[LANGUAGE_NONE][0]['value'] ?? '';
    $status_id = $node->field_status[LANGUAGE_NONE][0]['tid'] ?? bt_api_response(406, BT_API_TASK);
    $task['status'] = taxonomy_term_load($status_id)->name ?? bt_api_response(406, BT_API_TASK);
    $assignet_to_id = $node->field_assignet_to[LANGUAGE_NONE][0]['target_id'] ?? NULL;
    $task['assignet_to'] = bt_api_user($assignet_to_id) ?? bt_api_response(406, BT_API_TASK);
    $project_id = $node->field_project[LANGUAGE_NONE][0]['target_id'] ?? NULL;
    // Also get project.
    $task['project'] = project_get_by_id($project_id) ?? bt_api_response(406, BT_API_TASK);
    bt_api_headers();
    die(json_encode($task));
  }

  bt_api_response(406, BT_API_TASK);
}

/**
 * Check if user is member of project of task.
 */
function task_is_member_by_task_id($id) {
  global $user;
  $uid = bt_api_user_get();
  $user = user_load($uid);
  // Check if user have permissions to this task.
  $query = db_select('field_data_field_members', 'm');
  $query->leftJoin('field_data_field_project', 'p', 'm.entity_id = p.field_project_target_id');
  $query->condition('p.bundle', 'task');
  $query->fields('p', ['entity_id']);
  $query->condition('m.bundle', 'project');
  $query->condition('m.field_members_target_id', $user->uid);
  $query->condition('p.entity_id', $id);
  $result = $query->execute()->fetchAll();
  if (!empty($result)) {
    return $result;
  }
  bt_api_response(403, BT_API_TASK);
}

/**
 * Check if user is member of project of task.
 */
function task_is_member_by_project_id($id, $new = FALSE) {
  global $user;
  $uid = bt_api_user_get();
  $user = user_load($uid);
  // Check if user have permissions to this task.
  $query = db_select('field_data_field_members', 'm');
  if ($new === FALSE) {
    $query->leftJoin('field_data_field_project', 'p', 'm.entity_id = p.field_project_target_id');
    $query->condition('p.bundle', 'task');
    $query->fields('p', ['entity_id']);
  }
  else {
    $query->fields('m', ['entity_id']);
  }
  $query->condition('m.bundle', 'project');
  $query->condition('m.field_members_target_id', $user->uid);
  $query->condition('m.entity_id', $id);
  $result = $query->execute()->fetchAll();
  if (!empty($result)) {
    $tasks_ids = array_column($result, 'entity_id');
    return $tasks_ids;
  }
  bt_api_response(403, BT_API_TASK);
}
