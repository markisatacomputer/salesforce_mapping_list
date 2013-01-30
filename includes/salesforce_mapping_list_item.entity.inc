<?php

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingListItem extends Entity {

  public
    $salesforce_mapping_list_item_id,
    $salesforce_mapping_list_id,
    $salesforce_object_id,
    $salesforce_object_field,
    $entity_type = 'taxonomy_term',
    $entity_id,
    $created,
    $entity_updated,
    $last_sync;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_list_field');
  }
  
  public function delete() {
    //delete associated entity
    $entity = entity_load($this->entity_type, $this->entity_id);
    if (!empty($entity)){
      foreach($entity as $e) {
        $e->delete();
      }
    }

    parent::delete();
  }

  public function save() {
    if (isset($this->is_new) && $this->is_new) {
      $this->created = REQUEST_TIME;
    }
    if (!$this->entity_updated) {
      $this->entity_updated = REQUEST_TIME;
    }
    if (!$this->last_sync) {
      $this->last_sync = REQUEST_TIME;
    }
    return parent::save();
  }

}
