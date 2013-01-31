<?php

/**
 * Entity class for Salesforce Mappings
 */
class SalesforceMappingList extends Entity {

  public
    $salesforce_mapping_list_id,
    $name,
    $label,
    $type = 'salesforce_mapping_list', // only one bundle type for now
    $salesforce_object_type,
    $salesforce_object_field,
    $drupal_entity_type = 'taxonomy_term', // only mapping to taxonomy terms for now
    $drupal_bundle,
    $mapped_property = 'name', // only mapping to taxonomy terms for now
    $update_period,
    $created,
    $updated,
    $last_sync,
    $weight;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_list');
  }
  
  public function get_items($properties = array()) {
    //get all associated mapping_list_field entities
    $efq = new EntityFieldQuery();
    $efq->entityCondition('entity_type', 'salesforce_mapping_list_item')
        ->propertyCondition('salesforce_mapping_list_id', $this->salesforce_mapping_list_id);
    foreach ($properties as $key => $value) {
      $efq->propertyCondition($key, $value);
    }
    $results = $efq->execute();
    
    if (!empty($results) && !empty($results['salesforce_mapping_list_item'])) {
      return entity_load('salesforce_mapping_list_item', array_keys($results['salesforce_mapping_list_item']));
    } else {
      return FALSE;
    }
  }
  
  public function delete() {
    //delete associated mapping_list_items
    if ($fields = $this->get_items()) {
      foreach($fields as $field) {
        $field->delete();
      }
    }

    parent::delete();
  }

  public function save() {
    $this->updated = REQUEST_TIME;
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

  protected function defaultUri() {
    return array('path' => 'admin/structure/salesforce/list_mappings/manage/' . $this->identifier());
  }

}
