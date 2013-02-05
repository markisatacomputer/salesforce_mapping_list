<?php

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingListItem extends Entity {

  public
    $salesforce_mapping_list_item_id,
    $salesforce_mapping_list_id,
    $entity_type = 'taxonomy_term',
    $entity_id,
    $mapped_property = 'name',
    $created,
    $entity_updated;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'salesforce_mapping_list_item');
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
    return parent::save();
  }
  
  public function create_entity($data) {
    //  Do not create if entity already exists.
    if ($this->entity_id) {
      if ($this->entity() !== FALSE) {
        return FALSE;
      }
    }
    
    $entity_info = entity_get_info($this->drupal_entity_type);
    $values = array(
      $this->mapped_property => $data,
      $entity_info['entity keys']['bundle'] => $this->drupal_bundle,
    );
    $entity = entity_create($this->drupal_entity_type, $values);
    $wrapper = entity_metadata_wrapper($this->drupal_entity_type, $entity);
    $wrapper->save();
    
    $entity_values = $wrapper->value();
    $entity_id = $entity_values->$entity_info['entity keys']['id'];

    return $entity_id;
  }  
  
  public function get_mapping() {
    //get all associated mapping_list_field entities
    $efq = new EntityFieldQuery();
    $efq->entityCondition('entity_type', 'salesforce_mapping_list')
        ->propertyCondition('salesforce_mapping_list_id', $this->salesforce_mapping_list_id);
    $results = $efq->execute();
    
    if (!empty($results) && !empty($results['salesforce_mapping_list_item'])) {
      return entity_load('salesforce_mapping_list', array_keys($results['salesforce_mapping_list']));
    } else {
      return FALSE;
    }
  }
  
  public function entity() {
    $entity = entity_load($this->entity_type,array($this->entity_id));
    if (!empty($entity)) {
      return $entity[$this->entity_type];
    }
    return FALSE;
  }
  
  public function entity_property($property = 'name') {
    $entity = $this->entity();
    if ($entity && iseet($this->$property)) {
      return $this->$property;
    }
    return FALSE;
  }

}
