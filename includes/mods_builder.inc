<?php

/**
 * @file
 * NMODS builder class.
 */

class agile_mods_builder {

  const MADS_URI = "http://www.loc.gov/mods/v3";

  public function __construct($mods_xml = NULL) {
    $dom = new DomDocument('1.0', 'UTF-8');
    $dom->preserveWhiteSpace = false;
    $dom->formatOutput = true;
    if ($mods_xml) {
      $dom->loadXML($mods_xml);
      $this->root = $dom->documentElement;
    }
    else {
      $root = $dom->createElementNS(self::MADS_URI, 'mods:mods');
      $dom->appendChild($root);
      $this->root = $root;
    }
    $this->dom = $dom;
  }

  public function set_title($title) {
    $titleInfo_node = $this->dom->createElementNS(self::MADS_URI, "mods:titleInfo");
    $this->root->appendChild($titleInfo_node);
    $title_node = $this->dom->createElementNS(self::MADS_URI, 'mods:title', $title);
    $titleInfo_node->appendChild($title_node);
  }

  public function as_xml() {
    return $this->dom->saveXML();
  }

  public function set_author($name) {
    // name.
    $name_node = $this->dom->createElementNS(self::MADS_URI, 'mods:name');
    $name_node->setAttribute('type', 'personal');
    $this->root->appendChild($name_node);
    //namePart
    $name_part = $this->dom->createElementNS(self::MADS_URI, 'mods:namePart', $name);
    $name_node->appendChild($name_part);
    //role
    $role_node = $this->dom->createElementNS(self::MADS_URI, 'mods:role');
    $name_node->appendChild($role_node);
    $roleTerm_node = $this->dom->createElementNS(self::MADS_URI, 'mods:roleTerm', "Author");
    $roleTerm_node->setAttribute('authority', 'marcrelator');
    $roleTerm_node->setAttribute('type', 'text');
    $role_node->appendChild($roleTerm_node);
  }

  public function set_identifier($identifier, $type = 'local') {
    $identifier_node = $this->dom->createElementNS(self::MADS_URI, 'mods:identifier', $identifier);
    $identifier_node->setAttribute('type', $type);
    $this->root->appendChild($identifier_node);
  }

  public function add_as_datastream(&$object) {
    $mods = $this->as_xml();
    $mods_ds = $object->constructDatastream('MODS', 'M');
    $mods_ds->setContentFromString($mods);
    $mods_ds->label = 'MODS';
    $mods_ds->mimetype = 'text/xml';
    $object->ingestDatastream($mods_ds);
  }

}
