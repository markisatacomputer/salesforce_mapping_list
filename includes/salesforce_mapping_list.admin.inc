<?php

define('SALESFORCE_MAPPING_NAME_LENGTH', 128);

/**
 * Return a form for a salesforce mapping entity.
 *
 * @param $form
 * @param $form_state
 * @param null $mapping
 *
 * @return mixed
 */
function salesforce_mapping_list_form($form, &$form_state, SalesforceMappingList $mapping = NULL, $op = 'edit') {
  if ($op == 'clone') {
    $mapping->label .= ' (cloned)';
    $mapping->name = '';
  }
  $form['#id'] = 'salesforce_mapping_list_form';
  $form_state['salesforce_mapping'] = $mapping;
  
  $ajax = array(
    'callback' => 'salesforce_mapping_list_form_callback',
    'wrapper' => 'salesforce_mapping_list_form'
  );

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => isset($mapping) ? $mapping->label : '',
    '#description' => t('The human-readable name of this field mapping.'),
    '#required' => TRUE,
    '#maxlength' => SALESFORCE_MAPPING_NAME_LENGTH,
    '#size' => 30,
  );

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'machine_name',
    '#description' => t(''),
    '#default_value' => isset($mapping) ? $mapping->name : '',
    '#disabled' => !empty($mapping->name),
    '#machine_name' => array(
      'exists' => 'salesforce_mapping_list_load',
      'source' => array('label')
    ),
  );

  $form['drupal_entity'] = array(
    '#title' => t('Drupal entity'),
    '#type' => 'fieldset',
    '#attributes' => array(
      'id' => array('edit-drupal-entity'),
    ),
  );
  
  // These two might be incorporated later.
  $form['drupal_entity']['drupal_entity_type'] = array(
    '#type' => 'hidden',
    '#value' => isset($mapping) ? $mapping->drupal_entity_type : 0,
  );
  $form['drupal_entity']['mapped_property'] = array(
    '#type' => 'hidden',
    '#value' => isset($mapping) ? $mapping->mapped_property : 0,
  );

  $drupal_entity_type = & $form_state['values']['drupal_entity_type'];
  if (!$drupal_entity_type && isset($mapping)) {
    $drupal_entity_type = $mapping->drupal_entity_type;
  }
  if ($drupal_entity_type) {
    $bundles = array('' => t('-- Select --'));
    $info = entity_get_info($drupal_entity_type);
    foreach ($info['bundles'] as $key => $bundle) {
      $bundles[$key] = $bundle['label'];
    }
    $form['drupal_entity']['drupal_bundle'] = array(
      '#title' => t('Drupal Taxonomy'),// t('Drupal Entity Bundle'),
      '#type' => 'select',
      '#required' => TRUE,
      '#description' => t('Select a Drupal taxonomy in which this list will populate terms.'),
      '#options' => $bundles,
      '#default_value' => isset($mapping) ? $mapping->drupal_bundle : 0,
    );
  }

  $form['salesforce_object'] = array(
    '#title' => t('Salesforce object'),
    '#type' => 'fieldset',
    '#attributes' => array(
      'id' => array('edit-salesforce-object'),
    ),
  );

  $sfapi = salesforce_get_api();
  $sfobjects = & $form_state['sf_objects'];
  if (!$sfobjects) {
    $sfobjects = salesforce_mapping_list_valid_objects();
    $form_state['sf_objects'] = $sfobjects;
  }

  $sfobject_options = array('' => t('-- Select --'));
  foreach ($sfobjects as $key => $object) {
    $sfobject_options[$key] = $object['label'];
  }

  $form['salesforce_object']['salesforce_object_type'] = array(
    '#title' => t('Salesforce Object'),
    '#type' => 'select',
    '#description' => t('Select a Salesforce from which to choose a list.'),
    '#default_value' => isset($mapping) ? $mapping->salesforce_object_type : 0,
    '#options' => $sfobject_options,
    '#ajax' => $ajax,
    '#required' => TRUE
  );

  $salesforce_object_type = & $form_state['values']['salesforce_object_type'];
  if (!$salesforce_object_type && isset($mapping)) {
    $salesforce_object_type = $mapping->salesforce_object_type;
  }
  if (!empty($salesforce_object_type)) {
    $fields = $sfobjects[$salesforce_object_type]['fields'];
    $form['salesforce_object']['salesforce_object_field'] = array(
      '#title' => t('Salesforce Field List'),
      '#type' => 'select',
      '#description' => t('Select a Salesforce list field to import.'),
      '#default_value' => isset($mapping) ? $mapping->salesforce_object_field : 0,
      '#options' => $fields,
      '#required' => TRUE,
    );
  }
  
  $form['update_period'] = array(
    '#type' => 'select', 
    '#title' => t('Update Period'),
    '#description' => t('How often should the list be updated?'),
    '#default_value' => isset($mapping) ? $mapping->update_period : 604800,
    '#options' => salesforce_mapping_list_update_options(),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#value' => t('Save mapping'),
    '#type' => 'submit',
  );

  return $form;
}

/**
 * Ajax callback for salesforce_mapping_lists_form().
 */
function salesforce_mapping_list_form_callback($form, $form_state) {
  return $form;
}

/**
 * Validate salesforce_mapping_lists_form().
 *
 * @param $form
 * @param $form_state
 */
function salesforce_mapping_list_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $mapping = $form_state['salesforce_mapping'];

  // validate label and name length
  if (strlen($values['label']) > SALESFORCE_MAPPING_NAME_LENGTH) {
    form_set_error('label', t('Label must not exceed @max characters.', array('@max' => SALESFORCE_MAPPING_NAME_LENGTH)));
  }
  if (strlen($values['name']) > SALESFORCE_MAPPING_NAME_LENGTH) {
    form_set_error('name', t('Name must not exceed @max characters.', array('@max' => SALESFORCE_MAPPING_NAME_LENGTH)));
  }

  $efq = new EntityFieldQuery();
  $efq
    ->entityCondition('entity_type', 'salesforce_mapping_list')
    ->propertyCondition('salesforce_object_type', $values['salesforce_object_type'])
    ->propertyCondition('salesforce_object_field', $values['salesforce_object_field'])
    ->propertyCondition('drupal_entity_type', $values['drupal_entity_type'])
    ->propertyCondition('drupal_bundle', $values['drupal_bundle'])
    ->propertyCondition('mapped_property', $values['mapped_property'])
    ->count();

  // Existing mapping, ensure not using any other unique combo.
  if ($mapping->name) {
    $efq->propertyCondition('name', $mapping->name, '<>');
  }

  $count = $efq->execute();
  if ($count > 0) {
    form_set_error('drupal_bundle', t('This Drupal bundle has already been mapped to the salesforce list in question.'));
  }
}

/**
 *  Return list of salesforce objects with picklists.
 */
function salesforce_mapping_list_update_options() {
  $options = cache_get('salesforce_mapping_list_update_options');
  if ($options) {
    return $options->data;
  } else {
    $day = 86400;
    $week = 7*$day;
    $options = array();
    for ($i=1; $i<7; $i++) {
      $options[$i*$day] = "$i days";
    }
    for ($i=1; $i<5; $i++) {
      $options[$i*$week] = "$i weeks";
    }
    cache_set('salesforce_mapping_list_update_options', $options);
  }
}

/**
 * Submit handler for salesforce_mapping_lists_form().
 *
 * @param $form
 * @param $form_state
 */
function salesforce_mapping_list_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $mappings = array();

  $params = array(
    'label' => $values['label'],
    'salesforce_object_type' => $values['salesforce_object_type'],
    'salesforce_object_field' => $values['salesforce_object_field'],
    'drupal_entity_type' => $values['drupal_entity_type'],
    'drupal_bundle' => $values['drupal_bundle'],
    'update_period' => $values['update_period'],
    'description' => '',
  );

  if ($form_state['salesforce_mapping']->name) {
    $mapping = $form_state['salesforce_mapping'];
    foreach ($params as $key => $value) {
      $mapping->{$key} = $value;
    }
  }
  else {
    $params['name'] = $values['name'];
    $mapping = entity_create('salesforce_mapping_list', $params);
  }

  $mapping->save();

  drupal_set_message(t('Salesforce list field mapping saved.'));
  $form_state['redirect'] = 'admin/structure/salesforce/list_mappings';
}
