<?php

/**
 * @file
 * Migration fomrs and handlers.
 */

/**
 * Form to define parameters for moving volumes.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal from state
 *
 * @return array
 *   Drupal form
 */
function agile_migrate_volume_form($form, &$form_state) {
  $form = array();
  $form['source_dir'] = array(
    '#type' => 'textfield',
    '#title' => t("Volume Directory"),
    '#description' => t('Full path to volume directory'),
  );
  $form['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t("Namespace"),
    '#description' => t('Namespace for created collection'),
    '#default_value' => 'emic',
  );

  $form['collection'] = array(
    '#type' => 'textfield',
    '#title' => t("Collection"),
    '#description' => t('PID of collection to which volume is being added'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Ingest Volume"),
  );
  return $form;
}

/**
 * Validation handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_migrate_volume_form_validate($form, &$form_state) {
  if (!is_dir($form_state['values']['source_dir'])) {
    form_set_error('source_dir', t('Not a valid file path.'));
  }
  if (!islandora_object_load($form_state['values']['collection'])) {
    form_set_error('collection', t('Collection does not exist.'));
  }
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_migrate_volume_form_submit($form, &$form_state) {
  module_load_include('inc', 'agile_fonds_importer', 'includes/utilities');
  $extensions = array('tif', 'tiff');
  $tiffs = agile_get_all_by_extension($form_state['values']['source_dir'], $extensions);
  $batch = array(
    'title' => t('Adding Collection'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'agile_fonds_importer') . '/includes/agile_migration.inc',
    'progress_message' => t('@current of @total operations completed.'),
  );

  foreach ($tiffs as $tiff) {
    $batch['operations'][] = array('agile_batch_add_file', array(
        'file' => $tiff,
        'namespace' => $form_state['values']['namespace'],
        'collection_pid' => $form_state['values']['collection'],
      ),
    );
  }
  batch_set($batch);
  batch_process();
}

/**
 * Form to import csvs.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal from state
 *
 * @return array
 *   Drupal form
 */
function agile_import_csv_form($form, &$form_state) {
  $form = array();
  $form['source_file'] = array(
    '#type' => 'textfield',
    '#title' => t("Path to csv"),
    '#description' => t('Full path to volume directory'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Import CSV"),
  );
  return $form;
}

/**
 * Validation handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_import_csv_form_validate($form, &$form_state) {
  if (!is_file($form_state['values']['source_file']) && !is_dir($form_state['values']['source_file'])) {
    form_set_error('source_dir', t('Not a valid path.'));
  }
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal form state
 */
function agile_import_csv_form_submit($form, &$form_state) {
  module_load_include('inc', 'agile_fonds_importer', 'includes/utilities');
  if (is_file($form_state['values']['source_file'])) {
    $files = array($form_state['values']['source_file']);
  }
  else {
    $files = agile_get_all_by_extension($form_state['values']['source_file'], array('csv'));
  }
  foreach ($files as $csv_path) {
    agile_add_csv_to_database($csv_path);
  }
}

/**
 * Creates object from file.
 *
 * @global user $user
 *
 * @param string $file
 *   File name
 * @param string $namespace
 *   Namespace of ingested object
 * @param syring $collection_pid
 *   PID of collection
 * @param string $source_path
 *   File Path
 */
function agile_batch_add_file($file, $namespace, $collection_pid) {
  module_load_include('inc', 'agile_fonds_importer', 'includes/db');
  module_load_include('inc', 'agile_fonds_importer', 'includes/utilities');
  global $user;
  $path_info = pathinfo($file);
  $tuque = new IslandoraTuque();
  $stored_values = agile_read_metadata($path_info['filename'], "copy_negative_number");
  $provenence = agile_fonds_parse_item($stored_values['item_number']);
  $label = $path_info['filename'];
  $label = isset($stored_values['title']) ? $stored_values['title'] : $path_info['filename'];
  $object = $tuque->repository->constructObject($namespace);
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $collection_pid);
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:sp_large_image_cmodel');
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, HAS_FONDS_INDENTIFIER, $provenence['fonds'], RELS_TYPE_PLAIN_LITERAL);
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, HAS_VOLUME_INDENTIFIER, $provenence['volume'], RELS_TYPE_PLAIN_LITERAL);
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, HAS_FOLDER_INDENTIFIER, $provenence['folder'], RELS_TYPE_PLAIN_LITERAL);
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, HAS_FILE_INDENTIFIER, $provenence['file'], RELS_TYPE_PLAIN_LITERAL);
  $object->label = $metadata['title'];
  $object->owner = $user->name;
  // Add OBJ datasatream.
  $ds = $object->constructDatastream('OBJ', 'M');
  $ds->setContentFromFile($file, FALSE);
  $ds->label = 'OBJ';
  $ds->mimetype = "image/tiff";
  $object->ingestDatastream($ds);
  islandora_add_object($object);
  agile_update_with_pid($object->id, $stored_values['copy_negative_number']);
}
