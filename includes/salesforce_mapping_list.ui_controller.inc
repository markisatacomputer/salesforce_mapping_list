<?php

/**
 * Salesforce Mapping UI controller.
 */
class SalesforceMappingListUIController extends SalesforceMappingUIController {

  /**
   * Overrides hook_menu() defaults.
  */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage Salesforce List mappings';
    return $items;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableHeaders().
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header[] = t('Salesforce list field');
    $header = parent::overviewTableHeaders($conditions, $rows, $additional_header);
    return $header;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableRow().
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $additional_cols[] = $entity->salesforce_object_field;
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }
}
