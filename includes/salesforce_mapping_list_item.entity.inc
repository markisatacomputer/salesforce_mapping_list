<?php

/**
 * Entity class for Salesforce Mapping Objects
 */
class SalesforceMappingListItem extends Entity {

  public
    $salesforce_mapping_list_item_id,
    $salesforce_mapping_list_id,
    $entity_type = 'taxonomy_term',
    $entity_bundle,
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
    
    //  If entity with the values corresponding to $this exists, link to it instead of creating a new entity.
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', $this->entity_type)
        ->entityCondition('bundle', $this->entity_bundle)
        ->propertyCondition($this->mapped_property, $data);
    $results = $query->execute();
    if (!empty($results) && !empty($results[$this->entity_type])) {
      $existing = array_keys($results[$this->entity_type]);
      return array_shift($existing);
    }
    
    //  If neither of the above, create a new entity.
    $entity_info = entity_get_info($this->entity_type);
    $values = array(
      $this->mapped_property => $data,
      $entity_info['entity keys']['bundle'] => $this->entity_bundle,
    );
    $entity = entity_create($this->entity_type, $values);
    $wrapper = entity_metadata_wrapper($this->entity_type, $entity);
    $wrapper->save();
    
    $entity_values = $wrapper->value();
    $entity_id = $entity_values->$entity_info['entity keys']['id'];
    watchdog('salesforce_mapping_list','Created :entity_type entity id = :id, :mapped_property = :value', array(':entity_type' => $this->entity_type, ':id' =>  $entity_id, ':mapped_property' => $this->mapped_property, ':value' => $data));

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
