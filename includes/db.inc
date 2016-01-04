<?php

/**
 * @file
 * Database Helper Functions.
 */

/**
 * Adds metadata from CSV to database table
 * @param array $result
 *   Associative array of metadata values.
 */
function agile_add_metadata($result) {
  db_merge('agile_fonds_metadata')
      ->key(array('copy_negative_number' => $result['Copy negative number']))
      ->fields(array(
        'item_number' => $result['Item number on W.O.'],
        'title' => $result['Title'],
        'copy_negative_number' => $result['Copy negative number'],
      ))
      ->execute();
}

/**
 * Returns metadata from record identified by copy negative number
 *
 * @param string $cnn
 *  Copy Negativce Number
 *
 * @return array
 *   associative array of  values from the database
 */
function agile_read_metadata($value, $field = 'copy_negative_number') {
  $result = db_query("SELECT * from {agile_fonds_metadata} WHERE $field = :value", array(":value" => $value));
  return $result->fetchAssoc();
}

/**
 * Adds PID to record following successful addition.
 *
 * @param string $pid
 *   PID of new object
 * @param string $cnn
 *   Copy negative number for  CSV
 */
function agile_update_with_pid($pid, $cnn) {
  db_update('agile_fonds_metadata')
      ->fields(array('pid' => $pid))
      ->condition('copy_negative_number', $cnn)
      ->execute();
}

/**
 * Returns metadata from record identified by pid
 *
 * @param string $cnn
 *  Copy Negativce Number
 *
 * @return array
 *   associative array of  values from the database
 */
function agile_check_ingest($pid) {
  $result = db_query("SELECT * from {agile_fonds_metadata} WHERE pid = :pid", array(":pid" => $pid));
  return $result->fetchAssoc();
}

/**
 * Returns ingestion status from record identified by cnn
 *
 * @param string $cnn
 *  Copy Negative Number
 *
 * @return array
 *   associative array of  values from the database
 */
function agile_check_uningested($cnn) {
  $result = db_query("SELECT pid from {agile_fonds_metadata} WHERE copy_negative_number = :cnn", array(":cnn" => $cnn));
  $pid = $result->fetchfield(0);
  switch ($pid) {
    case FALSE :
      $status = "missing";
      break;

    case 'uningested' :
      $status = 'uningested';
      break;

    default:
      $status = 'ingested';
  }
  return $status;
}
