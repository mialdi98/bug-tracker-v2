<?php

/**
 * @file
 * Maintains task for api.
 */

/**
 * Add new task.
 */
function angular_callback() {
  $path = drupal_get_path('module', 'bt_master') . '/includes';
  $main_js_path = array_keys((array) file_scan_directory($path, '/(?=main)(.*?)(\.js)/'))[0];
  $polyfills_js_path = array_keys((array) file_scan_directory($path, '/(?=polyfills)(.*?)(\.js)/'))[0];
  $runtime_js_path = array_keys((array) file_scan_directory($path, '/(?=runtime)(.*?)(\.js)/'))[0];
  $scripts_js_path = array_keys((array) file_scan_directory($path, '/(?=scripts)(.*?)(\.js)/'))[0];
  $styles_css_path = array_keys((array) file_scan_directory($path, '/(?=styles)(.*?)(\.css)/'))[0];

  die(
   '<!doctype html>
    <html lang="en">
    <head>
      <meta charset="utf-8">
      <title>BT</title>
      <base href="/">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" type="image/x-icon" href="' . $path . '/' . 'favicon.ico">
    <link rel="stylesheet" href="' . $styles_css_path . '"></head>
    <body>
      <app-root></app-root>
    <script src="' . $runtime_js_path . '" defer></script>
    <script src="' . $polyfills_js_path . '" defer></script>
    <script src="' . $scripts_js_path . '" defer></script>
    <script src="' . $main_js_path . '" defer></script></body>
    </html>
  ');
}
