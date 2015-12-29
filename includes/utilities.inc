<?php

/**
 * @file
 * Utility functions.
 */
function agile_add_csv_to_database($csv_path) {
  module_load_include('inc', 'agile_fonds_importer', 'includes/db');
  $results = agile_fonds_read_csv($csv_path);
  foreach ($results as $result) {
    if ($result['Copy negative number']) {
      $success = agile_add_metadata($result);
    }
  }
}

/**
 * Parses provenence from filename.
 *
 * @param string $filename
 *   Full name of file
 *
 * @return array
 *   Parsed values.
 */
function agile_fonds_parse_file($filename) {
  $fonds = (substr_count($filename, 'MG30') > 0) ? 'MG30 D 21':'R2411';
  $balance = substr($filename, strlen($fonds));
  $parts = explode("-", $balance);
  $sub_parts = explode("_", $parts[1]);
  return array(
    'fonds' => $fonds,
    'volume' => $parts[0],
    'folder' => $sub_parts[0],
    'file' => $sub_parts[2],
  );
}

/**
 * Parses provenance from item  number.
 *
 * @param string $item
 *   Full item number from database
 * @return array
 *   Parsed values.
 */
function agile_fonds_parse_item($item) {
  $last = strrpos($item, " ");
  $fonds = substr($item, 0, $last);
  $local = substr($item, $last + 1);
  $box_bits = explode('-', $local);
  $volume = $box_bits[0];
  $folder_bits = explode('_', $box_bits[1]);
  $folder = $folder_bits[0];
  $file_identifier = $folder_bits[1];

  return array(
    'fonds' => $fonds,
    'volume' => $volume,
    'folder' => $folder,
    'file' => $file_identifier,
  );
}
/**
 * Reads csv.
 *
 * @param string $path
 *   Filepath to csv.
 *
 * @return array
 *   array of associative arrays of header => values
 */
function agile_fonds_read_csv($path) {
  ini_set("auto_detect_line_endings", "1");
  $results = array();
  if (($handle = fopen($path, "r")) !== FALSE) {
    while (($data = fgetcsv($handle, 10000, ",")) !== FALSE) {
      if (!isset($header)) {
        $header = $data;
        continue;
      }
      for ($counter = 0; $counter < count($data); $counter++) {
        $result[$header[$counter]] = $data[$counter];
      }
      if ($result['Title']) {
        $results[] = $result;
      }      
    }
    fclose($handle);
  }
  return $results;
}

/**
 * Gets all files with a given extension in a recursively searched directory.
 *
 * @param string $dir_path
 *   Full path to directory
 * @param array $extensions
 *   array of allowable extensions
 *
 * @return array
 *   All files with supllied extensions.
 */
function agile_get_all_by_extension($dir_path, $extensions) {
  $dirs = array();
  $files = array();
  foreach ($iterator = new RecursiveIteratorIterator(
  new RecursiveDirectoryIterator($dir_path, RecursiveDirectoryIterator::SKIP_DOTS), RecursiveIteratorIterator::SELF_FIRST) as $item) {
    $subpath = $iterator->getSubPathName();
    if ($item->isDir()) {
      $dirs[] = $subpath;
    }
    else {
      $ext = pathinfo($subpath, PATHINFO_EXTENSION);
      if (in_array($ext, $extensions)) {
        $files[] = "$dir_path/$subpath";
      }
    }
  }
  sort($files);
  return $files;
}
